/*
 *  This code started out as a version of arch/arm/common/dmabounce.c,
 *  modified to cope with highmem pages. Now it has been changed heavily -
 *  it now preallocates a large block (currently 4MB) and carves it up
 *  sequentially in ring fashion, and DMA is used to copy the data - to the
 *  point where very little of the original remains.
 *
 *  Copyright (C) 2019 Raspberry Pi (Trading) Ltd.
 *
 *  Original version by Brad Parker (brad@heeltoe.com)
 *  Re-written by Christopher Hoover <ch@murgatroid.com>
 *  Made generic by Deepak Saxena <dsaxena@plexity.net>
 *
 *  Copyright (C) 2002 Hewlett Packard Company.
 *  Copyright (C) 2004 MontaVista Software, Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/page-flags.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dma-direct.h>
#include <linux/dma-noncoherent.h>
#include <linux/dmapool.h>
#include <linux/list.h>
#include <linux/scatterlist.h>
#include <linux/bitmap.h>
#include <linux/swiotlb.h>

#include <asm/cacheflush.h>

#define STATS

#ifdef STATS
#define DO_STATS(X) do { X ; } while (0)
#else
#define DO_STATS(X) do { } while (0)
#endif

/* ************************************************** */

struct safe_buffer {
	struct list_head node;

	/* original request */
	size_t		size;
	int		direction;

	struct dmabounce_pool *pool;
	void		*safe;
	dma_addr_t	unsafe_dma_addr;
	dma_addr_t	safe_dma_addr;
};

struct dmabounce_pool {
	unsigned long	pages;
	void		*virt_addr;
	dma_addr_t	dma_addr;
	unsigned long	*alloc_map;
	unsigned long	alloc_pos;
	spinlock_t	lock;
	struct device	*dev;
	unsigned long   num_pages;
#ifdef STATS
	size_t		max_size;
	unsigned long	num_bufs;
	unsigned long   max_bufs;
	unsigned long   max_pages;
#endif
};

struct dmabounce_device_info {
	struct device *dev;
	dma_addr_t threshold;
	struct list_head safe_buffers;
	struct dmabounce_pool pool;
	rwlock_t lock;
#ifdef STATS
	unsigned long map_count;
	unsigned long unmap_count;
	unsigned long sync_dev_count;
	unsigned long sync_cpu_count;
	unsigned long fail_count;
	int attr_res;
#endif
};

static struct dmabounce_device_info *g_dmabounce_device_info;

extern int bcm2838_dma40_memcpy_init(void);
extern void bcm2838_dma40_memcpy(dma_addr_t dst, dma_addr_t src, size_t size);

#ifdef STATS
static ssize_t
bounce_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct dmabounce_device_info *device_info = g_dmabounce_device_info;
	return sprintf(buf, "m:%lu/%lu s:%lu/%lu f:%lu s:%zu b:%lu/%lu a:%lu/%lu\n",
		device_info->map_count,
		device_info->unmap_count,
		device_info->sync_dev_count,
		device_info->sync_cpu_count,
		device_info->fail_count,
		device_info->pool.max_size,
		device_info->pool.num_bufs,
		device_info->pool.max_bufs,
		device_info->pool.num_pages * PAGE_SIZE,
		device_info->pool.max_pages * PAGE_SIZE);
}

static DEVICE_ATTR(dmabounce_stats, 0444, bounce_show, NULL);
#endif

static int bounce_create(struct dmabounce_pool *pool, struct device *dev,
			 unsigned long buffer_size)
{
	int ret = -ENOMEM;
	pool->pages = (buffer_size + PAGE_SIZE - 1)/PAGE_SIZE;
	pool->alloc_map = bitmap_zalloc(pool->pages, GFP_KERNEL);
	if (!pool->alloc_map)
		goto err_bitmap;
	pool->virt_addr = dma_alloc_coherent(dev, pool->pages * PAGE_SIZE,
					     &pool->dma_addr, GFP_KERNEL);
	if (!pool->virt_addr)
		goto err_dmabuf;

	pool->alloc_pos = 0;
	spin_lock_init(&pool->lock);
	pool->dev = dev;
	pool->num_pages = 0;

	DO_STATS(pool->max_size = 0);
	DO_STATS(pool->num_bufs = 0);
	DO_STATS(pool->max_bufs = 0);
	DO_STATS(pool->max_pages = 0);

	return  0;

err_dmabuf:
	bitmap_free(pool->alloc_map);
err_bitmap:
	return ret;
}

static void bounce_destroy(struct dmabounce_pool *pool)
{
	dma_free_coherent(pool->dev, pool->pages * PAGE_SIZE, pool->virt_addr,
			  pool->dma_addr);

	bitmap_free(pool->alloc_map);
}

static void *bounce_alloc(struct dmabounce_pool *pool, size_t size,
			  dma_addr_t *dmaaddrp)
{
	unsigned long pages;
	unsigned long flags;
	unsigned long pos;

	pages = (size + PAGE_SIZE - 1)/PAGE_SIZE;

	DO_STATS(pool->max_size = max(size, pool->max_size));

	spin_lock_irqsave(&pool->lock, flags);
	pos = bitmap_find_next_zero_area(pool->alloc_map, pool->pages,
					 pool->alloc_pos, pages, 0);
	/* If not found, try from the start */
	if (pos >= pool->pages && pool->alloc_pos)
		pos = bitmap_find_next_zero_area(pool->alloc_map, pool->pages,
						 0, pages, 0);

	if (pos >= pool->pages) {
		spin_unlock_irqrestore(&pool->lock, flags);
		return NULL;
	}

	bitmap_set(pool->alloc_map, pos, pages);
	pool->alloc_pos = (pos + pages) % pool->pages;
	pool->num_pages += pages;

	DO_STATS(pool->num_bufs++);
	DO_STATS(pool->max_bufs = max(pool->num_bufs, pool->max_bufs));
	DO_STATS(pool->max_pages = max(pool->num_pages, pool->max_pages));

	spin_unlock_irqrestore(&pool->lock, flags);

	*dmaaddrp = pool->dma_addr + pos * PAGE_SIZE;

	return pool->virt_addr + pos * PAGE_SIZE;
}

static void
bounce_free(struct dmabounce_pool *pool, void *buf, size_t size)
{
	unsigned long pages;
	unsigned long flags;
	unsigned long pos;

	pages = (size + PAGE_SIZE - 1)/PAGE_SIZE;
	pos = (buf - pool->virt_addr)/PAGE_SIZE;

	BUG_ON((buf - pool->virt_addr) & (PAGE_SIZE - 1));

	spin_lock_irqsave(&pool->lock, flags);
	bitmap_clear(pool->alloc_map, pos, pages);
	pool->num_pages -= pages;
	if (pool->num_pages == 0)
		pool->alloc_pos = 0;
	DO_STATS(pool->num_bufs--);
	spin_unlock_irqrestore(&pool->lock, flags);
}

/* allocate a 'safe' buffer and keep track of it */
static struct safe_buffer *
alloc_safe_buffer(struct dmabounce_device_info *device_info,
		  dma_addr_t dma_addr, size_t size, enum dma_data_direction dir)
{
	struct safe_buffer *buf;
	struct dmabounce_pool *pool = &device_info->pool;
	struct device *dev = device_info->dev;
	unsigned long flags;

	/*
	 * Although one might expect this to be called in thread context,
	 * using GFP_KERNEL here leads to hard-to-debug lockups. in_atomic()
	 * was previously used to select the appropriate allocation mode,
	 * but this is unsafe.
	 */
	buf = kmalloc(sizeof(struct safe_buffer), GFP_ATOMIC);
	if (!buf) {
		dev_warn(dev, "%s: kmalloc failed\n", __func__);
		return NULL;
	}

	buf->unsafe_dma_addr = dma_addr;
	buf->size = size;
	buf->direction = dir;
	buf->pool = pool;

	buf->safe = bounce_alloc(pool, size, &buf->safe_dma_addr);

	if (!buf->safe) {
		dev_warn(dev,
			 "%s: could not alloc dma memory (size=%zu)\n",
			 __func__, size);
		kfree(buf);
		return NULL;
	}

	write_lock_irqsave(&device_info->lock, flags);
	list_add(&buf->node, &device_info->safe_buffers);
	write_unlock_irqrestore(&device_info->lock, flags);

	return buf;
}

/* determine if a buffer is from our "safe" pool */
static struct safe_buffer *
find_safe_buffer(struct dmabounce_device_info *device_info,
		 dma_addr_t safe_dma_addr)
{
	struct safe_buffer *b, *rb = NULL;
	unsigned long flags;

	read_lock_irqsave(&device_info->lock, flags);

	list_for_each_entry(b, &device_info->safe_buffers, node)
		if (b->safe_dma_addr <= safe_dma_addr &&
		    b->safe_dma_addr + b->size > safe_dma_addr) {
			rb = b;
			break;
		}

	read_unlock_irqrestore(&device_info->lock, flags);
	return rb;
}

static void
free_safe_buffer(struct dmabounce_device_info *device_info,
		 struct safe_buffer *buf)
{
	unsigned long flags;

	write_lock_irqsave(&device_info->lock, flags);
	list_del(&buf->node);
	write_unlock_irqrestore(&device_info->lock, flags);

	bounce_free(buf->pool, buf->safe, buf->size);

	kfree(buf);
}

/* ************************************************** */

static struct safe_buffer *
find_safe_buffer_dev(struct device *dev, dma_addr_t dma_addr, const char *where)
{
	if (!dev || !g_dmabounce_device_info)
		return NULL;
	if (dma_mapping_error(dev, dma_addr)) {
		dev_err(dev, "Trying to %s invalid mapping\n", where);
		return NULL;
	}
	return find_safe_buffer(g_dmabounce_device_info, dma_addr);
}

static dma_addr_t
map_single(struct device *dev, struct safe_buffer *buf, size_t size,
	   enum dma_data_direction dir, unsigned long attrs)
{
	BUG_ON(buf->size != size);
	BUG_ON(buf->direction != dir);

	dev_dbg(dev, "map: %llx->%llx\n", (u64)buf->unsafe_dma_addr,
		(u64)buf->safe_dma_addr);

	if ((dir == DMA_TO_DEVICE || dir == DMA_BIDIRECTIONAL) &&
	    !(attrs & DMA_ATTR_SKIP_CPU_SYNC))
		bcm2838_dma40_memcpy(buf->safe_dma_addr, buf->unsafe_dma_addr,
				     size);

	return buf->safe_dma_addr;
}

static dma_addr_t
unmap_single(struct device *dev, struct safe_buffer *buf, size_t size,
	     enum dma_data_direction dir, unsigned long attrs)
{
	BUG_ON(buf->size != size);
	BUG_ON(buf->direction != dir);

	if ((dir == DMA_FROM_DEVICE || dir == DMA_BIDIRECTIONAL) &&
	    !(attrs & DMA_ATTR_SKIP_CPU_SYNC)) {
		dev_dbg(dev, "unmap: %llx->%llx\n", (u64)buf->safe_dma_addr,
			(u64)buf->unsafe_dma_addr);

		bcm2838_dma40_memcpy(buf->unsafe_dma_addr, buf->safe_dma_addr,
				     size);
	}
	return buf->unsafe_dma_addr;
}

/* ************************************************** */

/*
 * see if a buffer address is in an 'unsafe' range.  if it is
 * allocate a 'safe' buffer and copy the unsafe buffer into it.
 * substitute the safe buffer for the unsafe one.
 * (basically move the buffer from an unsafe area to a safe one)
 */
static dma_addr_t
dmabounce_map_page(struct device *dev, struct page *page, unsigned long offset,
		   size_t size, enum dma_data_direction dir,
		   unsigned long attrs)
{
	struct dmabounce_device_info *device_info = g_dmabounce_device_info;
	dma_addr_t dma_addr;

	dma_addr = phys_to_dma(dev, page_to_phys(page)) + offset;

	dma_direct_sync_single_for_device(dev, dma_addr, size, dir);
        if (!dev_is_dma_coherent(dev))
		__dma_map_area(phys_to_virt(dma_to_phys(dev, dma_addr)), size, dir);

	if (device_info && (dma_addr + size) > device_info->threshold) {
		struct safe_buffer *buf;

		buf = alloc_safe_buffer(device_info, dma_addr, size, dir);
		if (!buf) {
			DO_STATS(device_info->fail_count++);
			return (~(dma_addr_t)0x0);
		}

		DO_STATS(device_info->map_count++);

		dma_addr = map_single(dev, buf, size, dir, attrs);
	}
	return dma_addr;
}

/*
 * see if a mapped address was really a "safe" buffer and if so, copy
 * the data from the safe buffer back to the unsafe buffer and free up
 * the safe buffer.  (basically return things back to the way they
 * should be)
 */
static void
dmabounce_unmap_page(struct device *dev, dma_addr_t dma_addr, size_t size,
		     enum dma_data_direction dir, unsigned long attrs)
{
	struct safe_buffer *buf;

	buf = find_safe_buffer_dev(dev, dma_addr, __func__);
	if (buf) {
		DO_STATS(g_dmabounce_device_info->unmap_count++);
		dma_addr = unmap_single(dev, buf, size, dir, attrs);
		free_safe_buffer(g_dmabounce_device_info, buf);
	}

        if (!dev_is_dma_coherent(dev))
		__dma_unmap_area(phys_to_virt(dma_to_phys(dev, dma_addr)), size, dir);
	dma_direct_sync_single_for_cpu(dev, dma_addr, size, dir);
}

/*
 * A version of dmabounce_map_page that assumes the mapping has already
 * been created - intended for streaming operation.
 */
static void
dmabounce_sync_for_device(struct device *dev, dma_addr_t dma_addr, size_t size,
			  enum dma_data_direction dir)
{
	struct safe_buffer *buf;

        dma_direct_sync_single_for_device(dev, dma_addr, size, dir);
        if (!dev_is_dma_coherent(dev))
                __dma_map_area(phys_to_virt(dma_to_phys(dev, dma_addr)), size, dir);

	buf = find_safe_buffer_dev(dev, dma_addr, __func__);
	if (buf) {
		DO_STATS(g_dmabounce_device_info->sync_dev_count++);
		map_single(dev, buf, size, dir, 0);
	}
}

/*
 * A version of dmabounce_unmap_page that doesn't destroy the mapping -
 * intended for streaming operation.
 */
static void
dmabounce_sync_for_cpu(struct device *dev, dma_addr_t dma_addr,
		       size_t size, enum dma_data_direction dir)
{
	struct safe_buffer *buf;

	buf = find_safe_buffer_dev(dev, dma_addr, __func__);
	if (buf) {
		DO_STATS(g_dmabounce_device_info->sync_cpu_count++);
		dma_addr = unmap_single(dev, buf, size, dir, 0);
	}

        if (!dev_is_dma_coherent(dev))
                __dma_unmap_area(phys_to_virt(dma_to_phys(dev, dma_addr)), size, dir);
        dma_direct_sync_single_for_cpu(dev, dma_addr, size, dir);
}

static int dmabounce_dma_supported(struct device *dev, u64 dma_mask)
{
	if (g_dmabounce_device_info)
		return 0;

	return dma_direct_supported(dev, dma_mask);
}

static const struct dma_map_ops dmabounce_ops = {
	.alloc			= dma_direct_alloc,
	.free			= dma_direct_free,
	.map_page		= dmabounce_map_page,
	.unmap_page		= dmabounce_unmap_page,
	.sync_single_for_cpu	= dmabounce_sync_for_cpu,
	.sync_single_for_device	= dmabounce_sync_for_device,
	.map_sg			= dma_direct_map_sg,
	.unmap_sg		= dma_direct_unmap_sg,
	.sync_sg_for_cpu	= dma_direct_sync_sg_for_cpu,
	.sync_sg_for_device	= dma_direct_sync_sg_for_device,
	.dma_supported		= dmabounce_dma_supported,
};

int brcm_pcie_bounce_init(struct device *dev,
			  unsigned long buffer_size,
			  dma_addr_t threshold)
{
	struct dmabounce_device_info *device_info;
	int ret;

	/* Only support a single client */
	if (g_dmabounce_device_info)
		return -EBUSY;

	ret = bcm2838_dma40_memcpy_init();
	if (ret)
		return ret;

	device_info = kmalloc(sizeof(struct dmabounce_device_info), GFP_ATOMIC);
	if (!device_info) {
		dev_err(dev,
			"Could not allocated dmabounce_device_info\n");
		return -ENOMEM;
	}

	ret = bounce_create(&device_info->pool, dev, buffer_size);
	if (ret) {
		dev_err(dev,
			"dmabounce: could not allocate %ld byte DMA pool\n",
			buffer_size);
		goto err_bounce;
	}

	device_info->dev = dev;
	device_info->threshold = threshold;
	INIT_LIST_HEAD(&device_info->safe_buffers);
	rwlock_init(&device_info->lock);

	DO_STATS(device_info->map_count = 0);
	DO_STATS(device_info->unmap_count = 0);
	DO_STATS(device_info->sync_dev_count = 0);
	DO_STATS(device_info->sync_cpu_count = 0);
	DO_STATS(device_info->fail_count = 0);
	DO_STATS(device_info->attr_res =
		 device_create_file(dev, &dev_attr_dmabounce_stats));

	g_dmabounce_device_info = device_info;

	dev_info(dev, "dmabounce: initialised - %ld kB, threshold %pad\n",
		 buffer_size / 1024, &threshold);

	return 0;

 err_bounce:
	kfree(device_info);
	return ret;
}
EXPORT_SYMBOL(brcm_pcie_bounce_init);

void brcm_pcie_bounce_uninit(struct device *dev)
{
	struct dmabounce_device_info *device_info = g_dmabounce_device_info;

	g_dmabounce_device_info = NULL;

	if (!device_info) {
		dev_warn(dev,
			 "Never registered with dmabounce but attempting"
			 "to unregister!\n");
		return;
	}

	if (!list_empty(&device_info->safe_buffers)) {
		dev_err(dev,
			"Removing from dmabounce with pending buffers!\n");
		BUG();
	}

	bounce_destroy(&device_info->pool);

	DO_STATS(if (device_info->attr_res == 0)
			 device_remove_file(dev, &dev_attr_dmabounce_stats));

	kfree(device_info);
}
EXPORT_SYMBOL(brcm_pcie_bounce_uninit);

int brcm_pcie_bounce_register_dev(struct device *dev)
{
	set_dma_ops(dev, &dmabounce_ops);

	return 0;
}
EXPORT_SYMBOL(brcm_pcie_bounce_register_dev);

MODULE_AUTHOR("Phil Elwell <phil@raspberrypi.org>");
MODULE_DESCRIPTION("Dedicate DMA bounce support for pcie-brcmstb");
MODULE_LICENSE("GPL");
