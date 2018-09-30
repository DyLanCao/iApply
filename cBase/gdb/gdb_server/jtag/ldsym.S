/*
 * ldsym.S
 *
 * Copyright (C) 2005
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Thomas Klein <ThRKlein@sf.net>, 2005.
 *
 */

#if defined(__ELF__)

.section .data
.global dcc_fl_amd8_start
.global dcc_fl_amd8_size
.global dcc_fl_amd16_start
.global dcc_fl_amd16_size
.global dcc_fl_amd32_start
.global dcc_fl_amd32_size
.global dcc_fl_philips_start
.global dcc_fl_philips_size
.global dcc_fl_atmel_start
.global dcc_fl_atmel_size
.global dcc_fl_st_start
.global dcc_fl_st_size
.global dcc_read_start
.global dcc_read_size
.global dcc_write_start
.global dcc_write_size
.global dcc_check_start
.global dcc_check_size
.global dcc_dummy_start
.global dcc_dummy_size
dcc_fl_amd8_start:		.long _binary_fl_amd8_bin_start
dcc_fl_amd8_size:		.long _binary_fl_amd8_bin_size
dcc_fl_amd16_start:		.long _binary_fl_amd16_bin_start
dcc_fl_amd16_size:		.long _binary_fl_amd16_bin_size
dcc_fl_amd32_start:		.long _binary_fl_amd32_bin_start
dcc_fl_amd32_size:		.long _binary_fl_amd32_bin_size
dcc_fl_philips_start:		.long _binary_fl_philips_bin_start
dcc_fl_philips_size:		.long _binary_fl_philips_bin_size
dcc_fl_atmel_start:		.long _binary_fl_atmel_bin_start
dcc_fl_atmel_size:		.long _binary_fl_atmel_bin_size
dcc_fl_st_start:		.long _binary_fl_st_bin_start
dcc_fl_st_size:			.long _binary_fl_st_bin_size
dcc_read_start:			.long _binary_dcc_read_bin_start
dcc_read_size:			.long _binary_dcc_read_bin_size
dcc_write_start:		.long _binary_dcc_write_bin_start
dcc_write_size:			.long _binary_dcc_write_bin_size
dcc_check_start:		.long _binary_dcc_check_bin_start
dcc_check_size:			.long _binary_dcc_check_bin_size
dcc_dummy_start:		.long _binary_dcc_dummy_bin_start
dcc_dummy_size:			.long _binary_dcc_dummy_bin_size

#else // ! __ELF__

.section .data
.global _dcc_fl_amd8_start
.global _dcc_fl_amd8_size
.global _dcc_fl_amd16_start
.global _dcc_fl_amd16_size
.global _dcc_fl_amd32_start
.global _dcc_fl_amd32_size
.global _dcc_fl_philips_start
.global _dcc_fl_philips_size
.global _dcc_fl_atmel_start
.global _dcc_fl_atmel_size
.global _dcc_fl_st_start
.global _dcc_fl_st_size
.global _dcc_read_start
.global _dcc_read_size
.global _dcc_write_start
.global _dcc_write_size
.global _dcc_check_start
.global _dcc_check_size
.global _dcc_dummy_start
.global _dcc_dummy_size
_dcc_fl_amd8_start:		.long _binary_fl_amd8_bin_start
_dcc_fl_amd8_size:		.long _binary_fl_amd8_bin_size
_dcc_fl_amd16_start:		.long _binary_fl_amd16_bin_start
_dcc_fl_amd16_size:		.long _binary_fl_amd16_bin_size
_dcc_fl_amd32_start:		.long _binary_fl_amd32_bin_start
_dcc_fl_amd32_size:		.long _binary_fl_amd32_bin_size
_dcc_fl_philips_start:		.long _binary_fl_philips_bin_start
_dcc_fl_philips_size:		.long _binary_fl_philips_bin_size
_dcc_fl_atmel_start:		.long _binary_fl_atmel_bin_start
_dcc_fl_atmel_size:		.long _binary_fl_atmel_bin_size
_dcc_fl_st_start:		.long _binary_fl_st_bin_start
_dcc_fl_st_size:		.long _binary_fl_st_bin_size
_dcc_read_start:		.long _binary_dcc_read_bin_start
_dcc_read_size:			.long _binary_dcc_read_bin_size
_dcc_write_start:		.long _binary_dcc_write_bin_start
_dcc_write_size:		.long _binary_dcc_write_bin_size
_dcc_check_start:		.long _binary_dcc_check_bin_start
_dcc_check_size:		.long _binary_dcc_check_bin_size
_dcc_dummy_start:		.long _binary_dcc_dummy_bin_start
_dcc_dummy_size:		.long _binary_dcc_dummy_bin_size
#endif
