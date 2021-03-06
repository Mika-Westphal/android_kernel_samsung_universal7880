/* linux/drivers/video/exynos/decon/vpp/vpp_regs.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung EXYNOS5 SoC series G-scaler driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License,
 * or (at your option) any later version.
 */

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include "vpp.h"
#include "vpp_common.h"
#include "vpp_coef.h"

int vpp_reg_wait_op_status(u32 id)
{
	u32 cfg = 0;

	unsigned long cnt = 100000;

	do {
		cfg = vpp_read(id, VG_ENABLE);
		if (!(cfg & (VG_ENABLE_OP_STATUS)))
			return 0;
		udelay(10);
	} while (--cnt);

	vpp_err("timeout op_status to idle\n");

	return -EBUSY;
}

void vpp_reg_wait_idle(u32 id)
{
	u32 cfg = 0;

	unsigned long cnt = 100000;

	do {
		cfg = vpp_read(id, VG_ENABLE);
		if (!(cfg & (VG_ENABLE_OP_STATUS)))
			return;
		vpp_info("vpp%d is operating...\n", id);
		udelay(10);
	} while (--cnt);

	vpp_err("timeout op_status to idle\n");
}

int vpp_reg_set_sw_reset(u32 id)
{
	u32 cfg = 0;

	unsigned long cnt = 100000;
	vpp_write_mask(id, VG_ENABLE, ~0, VG_ENABLE_SRESET);

	do {
		cfg = vpp_read(id, VG_ENABLE);
		if (!(cfg & (VG_ENABLE_SRESET)))
			return 0;
		udelay(10);
	} while (--cnt);

	vpp_err("timeout sw reset\n");

	return -EBUSY;
}

void vpp_reg_wait_pingpong_clear(u32 id)
{
	u32 cfg = 0;
	unsigned long cnt = 1700;

	do {
		cfg = vpp_read(id, VG_PINGPONG_UPDATE);
		if (!(cfg & (VG_ADDR_PINGPONG_UPDATE)))
			return;
		udelay(10);
	} while (--cnt);
	vpp_err("timeout of VPP(%d) pingpong_clear\n", id);
}

void vpp_reg_set_deadlock_num(u32 id, u32 num)
{
	vpp_write(id, VG_DEADLOCK_NUM, num);
}

void vpp_reg_set_framedone_irq(u32 id, u32 enable)
{
	u32 val = enable ? ~0 : 0;
	vpp_write_mask(id, VG_IRQ, val, VG_IRQ_FRAMEDONE_MASK);
}

void vpp_reg_set_deadlock_irq(u32 id, u32 enable)
{
	u32 val = enable ? ~0 : 0;
	vpp_write_mask(id, VG_IRQ, val, VG_IRQ_DEADLOCK_STATUS_MASK);
}

void vpp_reg_set_read_slave_err_irq(u32 id, u32 enable)
{
	u32 val = enable ? ~0 : 0;
	vpp_write_mask(id, VG_IRQ, val, VG_IRQ_READ_SLAVE_ERROR_MASK);
}

void vpp_reg_set_sfr_update_force(u32 id)
{
	vpp_write_mask(id, VG_ENABLE, ~0, VG_ENABLE_SFR_UPDATE_FORCE);
}

void vpp_reg_set_enable_interrupt(u32 id)
{
	vpp_write_mask(id, VG_IRQ, ~0, VG_IRQ_ENABLE);
}

void vpp_reg_set_hw_reset_done_mask(u32 id, u32 enable)
{
	u32 val = enable ? ~0 : 0;
	vpp_write_mask(id, VG_IRQ, val, VG_IRQ_HW_RESET_DONE_MASK);
}

int vpp_reg_set_in_format(u32 id, struct vpp_img_format *vi)
{
	u32 cfg = vpp_read(id, VG_IN_CON);

	cfg &= ~(VG_IN_CON_IMG_FORMAT_MASK | VG_IN_CON_CHROMINANCE_STRIDE_EN);

	if ((id == 0 || id == 1 || id == 2) &&
			(vi->format >= DECON_PIXEL_FORMAT_NV16)) {
		vpp_err("Unsupported YUV format%d in G%d \n", vi->format, id);
		return -EINVAL;
	}

	if (!vi->vgr && vi->afbc_en) {
		vpp_err("Unsupported AFBC format decoding in VPP%d\n", id);
		return -EINVAL;
	}

	if (!vi->vgr && (vi->scale || vi->rot)) {
		vpp_err("Unsupported Scailing in (V)G%d\n", id);
		return -EINVAL;
	}

	switch (vi->format) {
	case DECON_PIXEL_FORMAT_ARGB_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_ARGB8888;
		break;
	case DECON_PIXEL_FORMAT_ABGR_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_ABGR8888;
		break;
	case DECON_PIXEL_FORMAT_RGBA_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_RGBA8888;
		break;
	case DECON_PIXEL_FORMAT_BGRA_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_BGRA8888;
		break;
	case DECON_PIXEL_FORMAT_XRGB_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_XRGB8888;
		break;
	case DECON_PIXEL_FORMAT_XBGR_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_XBGR8888;
		break;
	case DECON_PIXEL_FORMAT_RGBX_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_RGBX8888;
		break;
	case DECON_PIXEL_FORMAT_BGRX_8888:
		cfg |= VG_IN_CON_IMG_FORMAT_BGRX8888;
		break;
	case DECON_PIXEL_FORMAT_RGB_565:
		cfg |= VG_IN_CON_IMG_FORMAT_RGB565;
		break;
	case DECON_PIXEL_FORMAT_NV16:
		cfg |= VG_IN_CON_IMG_FORMAT_YUV422_2P;
		break;
	case DECON_PIXEL_FORMAT_NV61:
		cfg |= VG_IN_CON_IMG_FORMAT_YVU422_2P;
		break;
	case DECON_PIXEL_FORMAT_NV12:
	case DECON_PIXEL_FORMAT_NV12M:
		cfg |= VG_IN_CON_IMG_FORMAT_YUV420_2P;
		break;
	case DECON_PIXEL_FORMAT_NV21:
	case DECON_PIXEL_FORMAT_NV21M:
	case DECON_PIXEL_FORMAT_NV12N:
		cfg |= VG_IN_CON_IMG_FORMAT_YVU420_2P;
		break;
	default:
		vpp_err("Unsupported Format\n");
		return -EINVAL;
	}

	vpp_write(id, VG_IN_CON, cfg);

	return 0;
}

void vpp_reg_set_in_buf_addr(u32 id, struct vpp_size_param *p, struct vpp_img_format *vi)
{
	vpp_dbg("y : %llu, cb : %llu, cr : %llu\n",
			p->addr0, p->addr1, p->addr2);

	vpp_write(id, VG_BASE_ADDR_Y(0), p->addr0);
	/* When processing the AFBC data, BASE_ADDR_Y and
	 * BASE_ADDR_CB should be set to the same address.
	 */
	if (vi->afbc_en == 0)
		vpp_write(id, VG_BASE_ADDR_CB(0), p->addr1);
	else
		vpp_write(id, VG_BASE_ADDR_CB(0), p->addr0);

	vpp_write(id, VG_PINGPONG_UPDATE, VG_ADDR_PINGPONG_UPDATE);
}

void vpp_reg_set_in_size(u32 id, struct vpp_size_param *p)
{
	u32 cfg = 0;

	/* source offset */
	cfg = VG_SRC_OFFSET_X(p->src_x) | VG_SRC_OFFSET_Y(p->src_y);
	vpp_write(id, VG_SRC_OFFSET, cfg);

	/* source full(alloc) size */
	cfg = VG_SRC_SIZE_WIDTH(p->src_fw) | VG_SRC_SIZE_HEIGHT(p->src_fh);
	vpp_write(id, VG_SRC_SIZE, cfg);

	/* source cropped size */
	cfg = VG_IMG_SIZE_WIDTH(p->src_w) | VG_IMG_SIZE_HEIGHT(p->src_h);
	vpp_write(id, VG_IMG_SIZE, cfg);
}

void vpp_reg_set_in_block_size(u32 id, u32 enable, struct vpp_size_param *p)
{
	u32 cfg = 0;

	if (!enable) {
		vpp_write_mask(id, VG_IN_CON, 0, VG_IN_CON_BLOCKING_FEATURE_EN);
		return;
	}

	/* blocking area offset */
	cfg = VG_BLK_OFFSET_X(p->block_x) | VG_BLK_OFFSET_Y(p->block_y);
	vpp_write(id, VG_BLK_OFFSET, cfg);

	/* blocking area size */
	cfg = VG_BLK_SIZE_WIDTH(p->block_w) | VG_BLK_SIZE_HEIGHT(p->block_h);
	vpp_write(id, VG_BLK_SIZE, cfg);

	vpp_write_mask(id, VG_IN_CON, ~0, VG_IN_CON_BLOCKING_FEATURE_EN);

	vpp_dbg("block x : %d, y : %d, w : %d, h : %d\n",
			p->block_x, p->block_y, p->block_w, p->block_h);
}

void vpp_reg_set_rgb_type(u32 id, u32 type)
{
	u32 csc_eq = 0;

	switch (type) {
	case BT_601_NARROW:
		csc_eq = VG_OUT_CON_RGB_TYPE_601_NARROW;
		break;
	case BT_601_WIDE:
		csc_eq = VG_OUT_CON_RGB_TYPE_601_WIDE;
		break;
	case BT_709_NARROW:
		csc_eq = VG_OUT_CON_RGB_TYPE_709_NARROW;
		break;
	case BT_709_WIDE:
		csc_eq = VG_OUT_CON_RGB_TYPE_709_WIDE;
		break;
	default:
		vpp_err("Unsupported CSC Equation\n");
	}

	vpp_write_mask(id, VG_IN_CON, csc_eq, VG_OUT_CON_RGB_TYPE_MASK);
}

void vpp_reg_set_plane_alpha(u32 id, u32 plane_alpha)
{
	if (plane_alpha > 0xFF)
		vpp_info("%d is too much value\n", plane_alpha);
	vpp_write_mask(id, VG_OUT_CON, VG_OUT_CON_FRAME_ALPHA(plane_alpha),
			VG_OUT_CON_FRAME_ALPHA_MASK);
}

void vpp_reg_set_plane_alpha_fixed(u32 id)
{
	vpp_write_mask(id, VG_OUT_CON, VG_OUT_CON_FRAME_ALPHA(0xFF),
			VG_OUT_CON_FRAME_ALPHA_MASK);
}

void vpp_reg_set_lookup_table(u32 id)
{
	vpp_write(id, VG_QOS_LUT07_00, 0x44444444);
	vpp_write(id, VG_QOS_LUT15_08, 0x44444444);
}

void vpp_reg_set_dynamic_clock_gating(u32 id)
{
	vpp_write(id, VG_DYNAMIC_GATING_ENABLE, 0x3F);
}

int vpp_reg_get_irq_status(u32 id)
{
	u32 cfg = vpp_read(id, VG_IRQ);
	cfg &= (VG_IRQ_HW_RESET_DONE | VG_IRQ_READ_SLAVE_ERROR |
		       VG_IRQ_DEADLOCK_STATUS |	VG_IRQ_FRAMEDONE);
	return cfg;
}

void vpp_reg_set_clear_irq(u32 id, u32 irq)
{
	vpp_write_mask(id, VG_IRQ, ~0, irq);
}

void vpp_constraints_params(struct vpp_size_constraints *vc,
		struct vpp_img_format *vi)
{
	if (!vi->wb && !vi->vgr) {
		if (vi->yuv) {
			vc->src_mul_w = YUV_SRC_SIZE_MULTIPLE;
			vc->src_mul_h = YUV_SRC_SIZE_MULTIPLE;
			vc->src_w_min = YUV_SRC_WIDTH_MIN;
			vc->src_w_max = YUV_SRC_WIDTH_MAX;
			vc->src_h_min = YUV_SRC_HEIGHT_MIN;
			vc->src_h_max = YUV_SRC_HEIGHT_MAX;
			vc->img_mul_w = YUV_IMG_SIZE_MULTIPLE;
			vc->img_mul_h = YUV_IMG_SIZE_MULTIPLE;
			vc->img_w_min = YUV_IMG_WIDTH_MIN;
			vc->img_w_max = IMG_WIDTH_MAX;
			vc->img_h_min = YUV_IMG_HEIGHT_MIN;
			vc->img_h_max = IMG_WIDTH_MAX;
			vc->src_mul_x = YUV_SRC_OFFSET_MULTIPLE;
			vc->src_mul_y = YUV_SRC_OFFSET_MULTIPLE;
			vc->sca_w_min = SCALED_WIDTH_MIN;
			vc->sca_w_max = SCALED_WIDTH_MAX;
			vc->sca_h_min = SCALED_HEIGHT_MIN;
			vc->sca_h_max = SCALED_HEIGHT_MAX;
			vc->sca_mul_w = SCALED_SIZE_MULTIPLE;
			vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
		} else {
			vc->src_mul_w = RGB_SRC_SIZE_MULTIPLE;
			vc->src_mul_h = RGB_SRC_SIZE_MULTIPLE;
			vc->src_w_min = RGB_SRC_WIDTH_MIN;
			vc->src_w_max = RGB_SRC_WIDTH_MAX;
			vc->src_h_min = RGB_SRC_HEIGHT_MIN;
			vc->src_h_max = RGB_SRC_HEIGHT_MAX;
			vc->img_mul_w = RGB_IMG_SIZE_MULTIPLE;
			vc->img_mul_h = RGB_IMG_SIZE_MULTIPLE;
			vc->img_w_min = RGB_IMG_WIDTH_MIN;
			vc->img_w_max = IMG_WIDTH_MAX;
			vc->img_h_min = RGB_IMG_HEIGHT_MIN;
			vc->img_h_max = IMG_WIDTH_MAX;
			vc->src_mul_x = RGB_SRC_OFFSET_MULTIPLE;
			vc->src_mul_y = RGB_SRC_OFFSET_MULTIPLE;
			vc->sca_w_min = SCALED_WIDTH_MIN;
			vc->sca_w_max = SCALED_WIDTH_MAX;
			vc->sca_h_min = SCALED_HEIGHT_MIN;
			vc->sca_h_max = SCALED_HEIGHT_MAX;
			vc->sca_mul_w = SCALED_SIZE_MULTIPLE;
			vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
		}
	} else if (!vi->wb && vi->vgr) {
		if (!vi->yuv) {
			vc->src_mul_w = RGB_SRC_SIZE_MULTIPLE;
			vc->src_mul_h = RGB_SRC_SIZE_MULTIPLE;
			vc->src_w_max = RGB_SRC_WIDTH_MAX;
			vc->src_h_max = RGB_SRC_HEIGHT_MAX;
			vc->sca_w_min = SCALED_WIDTH_MIN;
			vc->sca_w_max = SCALED_WIDTH_MAX;
			vc->sca_h_min = SCALED_HEIGHT_MIN;
			vc->sca_h_max = SCALED_HEIGHT_MAX;
			vc->src_mul_x = RGB_SRC_OFFSET_MULTIPLE;
			vc->src_mul_y = RGB_SRC_OFFSET_MULTIPLE;
			vc->sca_mul_w = SCALED_SIZE_MULTIPLE;
			vc->img_mul_w = PRE_RGB_WIDTH;
			vc->img_mul_h = PRE_RGB_HEIGHT;

			if (!vi->rot) {
				vc->src_w_min = ROT1_RGB_SRC_WIDTH_MIN;
				vc->src_h_min = ROT1_RGB_SRC_HEIGHT_MIN;
				vc->img_w_min = ROT1_RGB_IMG_WIDTH_MIN;
				vc->img_h_min = ROT1_RGB_IMG_HEIGHT_MIN;
				vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
			} else {
				vc->src_w_min = ROT2_RGB_SRC_WIDTH_MIN;
				vc->src_h_min = ROT2_RGB_SRC_HEIGHT_MIN;
				vc->img_w_min = ROT2_RGB_IMG_WIDTH_MIN;
				vc->img_h_min = ROT2_RGB_IMG_HEIGHT_MIN;
				vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
			}
			if (vi->normal) {
				vc->img_w_max = ROT3_RGB_IMG_WIDTH_MAX;
				vc->img_h_max = ROT3_RGB_IMG_HEIGHT_MAX;
			} else {
				vc->img_w_max = ROT4_RGB_IMG_WIDTH_MAX;
				vc->img_h_max = ROT4_RGB_IMG_HEIGHT_MAX;
				vc->blk_w_min = ROT3_RGB_BLK_WIDTH_MIN;
				vc->blk_w_max = ROT3_RGB_BLK_WIDTH_MAX;
				vc->blk_h_min = ROT3_RGB_BLK_HEIGHT_MIN;
				vc->blk_h_max = ROT3_RGB_BLK_HEIGHT_MAX;
			}
		} else {
			vc->src_mul_w = YUV_SRC_SIZE_MULTIPLE;
			vc->src_w_max = YUV_SRC_WIDTH_MAX;
			vc->src_h_max = YUV_SRC_HEIGHT_MAX;
			vc->sca_w_min = SCALED_WIDTH_MIN;
			vc->sca_w_max = SCALED_WIDTH_MAX;
			vc->sca_h_min = SCALED_HEIGHT_MIN;
			vc->sca_h_max = SCALED_HEIGHT_MAX;
			vc->src_mul_x = SRC_SIZE_MULTIPLE;
			vc->sca_mul_w = SCALED_SIZE_MULTIPLE;
			vc->img_mul_w = PRE_YUV_WIDTH;

			if (!vi->rot) {
				vc->src_w_min = ROT1_YUV_SRC_WIDTH_MIN;
				vc->src_h_min = ROT1_YUV_SRC_HEIGHT_MIN;
				vc->img_w_min = ROT1_YUV_IMG_WIDTH_MIN;
				vc->img_h_min = ROT1_YUV_IMG_HEIGHT_MIN;
			} else {
				vc->src_w_min = ROT2_YUV_SRC_WIDTH_MIN;
				vc->src_h_min = ROT2_YUV_SRC_HEIGHT_MIN;
				vc->img_w_min = ROT2_YUV_IMG_WIDTH_MIN;
				vc->img_h_min = ROT2_YUV_IMG_HEIGHT_MIN;
			}
			if (vi->normal) {
				vc->img_w_max = ROT3_YUV_IMG_WIDTH_MAX;
				vc->img_h_max = ROT3_YUV_IMG_HEIGHT_MAX;
			} else {
				vc->img_w_max = ROT4_YUV_IMG_WIDTH_MAX;
				vc->img_h_max = ROT4_YUV_IMG_HEIGHT_MAX;
			}
			if (vi->yuv422) {
				vc->src_mul_h = YUV_SRC_SIZE_MUL_HEIGHT;
				vc->img_mul_h = PRE_YUV_HEIGHT;
				if (!vi->rot) {
					vc->img_mul_h = PRE_ROT1_YUV_HEIGHT;
					vc->src_mul_y = YUV_SRC_SIZE_MUL_HEIGHT;
					vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
				} else {
					vc->src_mul_y = YUV_SRC_OFFSET_MULTIPLE;
					vc->img_mul_h = PRE_YUV_HEIGHT;
					vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
				}

			} else {
				vc->src_mul_h = YUV_SRC_SIZE_MULTIPLE;
				vc->img_mul_h = PRE_YUV_HEIGHT;
				vc->src_mul_y = YUV_SRC_OFFSET_MULTIPLE;
				vc->sca_mul_h = SCALED_SIZE_MULTIPLE;
			}
		}
	} else { /* write-back case */
		vc->src_mul_w = DST_SIZE_MULTIPLE;
		vc->src_mul_h = DST_SIZE_MULTIPLE;
		vc->src_w_min = DST_SIZE_WIDTH_MIN;
		vc->src_w_max = DST_SIZE_WIDTH_MAX;
		vc->src_h_min = DST_SIZE_HEIGHT_MIN;
		vc->src_h_max = DST_SIZE_HEIGHT_MAX;
		vc->img_mul_w = DST_IMGAGE_MULTIPLE;
		vc->img_mul_h = DST_IMGAGE_MULTIPLE;
		vc->img_w_min = DST_IMG_WIDTH_MIN;
		vc->img_w_max = DST_IMG_MAX;
		vc->img_h_min = DST_IMG_HEIGHT_MIN;
		vc->img_h_max = DST_IMG_MAX;
		vc->sca_w_min = DST_SIZE_WIDTH_MIN;
		vc->sca_w_max = DST_SIZE_WIDTH_MAX;
		vc->sca_h_min = DST_SIZE_HEIGHT_MIN;
		vc->sca_h_max = DST_SIZE_HEIGHT_MAX;
		vc->src_mul_x = DST_OFFSET_MULTIPLE;
		vc->src_mul_y = DST_OFFSET_MULTIPLE;
		vc->sca_mul_w = DST_OFFSET_MULTIPLE;
		vc->sca_mul_h = DST_OFFSET_MULTIPLE;
	}
}

void vpp_reg_init(u32 id)
{
	vpp_reg_set_sw_reset(id);
	vpp_reg_set_framedone_irq(id, false);
	vpp_reg_set_deadlock_irq(id, false);
	vpp_reg_set_read_slave_err_irq(id, false);
	vpp_reg_set_hw_reset_done_mask(id, false);
	vpp_reg_set_enable_interrupt(id);
	vpp_reg_set_lookup_table(id);
	vpp_reg_set_dynamic_clock_gating(id);
	vpp_reg_set_plane_alpha_fixed(id);
}

void vpp_reg_deinit(u32 id, u32 reset_en)
{
	unsigned int vpp_irq = 0;

	vpp_irq = vpp_reg_get_irq_status(id);
	vpp_reg_set_clear_irq(id, vpp_irq);

	vpp_reg_set_framedone_irq(id, true);
	vpp_reg_set_deadlock_irq(id, true);
	vpp_reg_set_read_slave_err_irq(id, true);
	vpp_reg_set_hw_reset_done_mask(id, true);
	if (reset_en)
		vpp_reg_set_sw_reset(id);
}
