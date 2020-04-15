/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdint.h>
#include <stdlib.h>
#include <syslog.h>
#include "interrupt.h"
#include "dump.h"

static const char* TAG = "INTERRUPT";

void __attribute__((weak))
handle_irq_dummy(uintptr_t cause, uintptr_t epc)
{
    LOGE(TAG, "unhandled interrupt: Cause 0x%016lx, EPC 0x%016lx\n", cause, epc);
    exit(1337);
}

void __attribute__((weak, alias("handle_irq_dummy")))
handle_irq_m_soft(uintptr_t cause, uintptr_t epc);

void __attribute__((weak, alias("handle_irq_dummy")))
handle_irq_m_timer(uintptr_t cause, uintptr_t epc);

void __attribute__((weak, alias("handle_irq_dummy")))
handle_irq_m_ext(uintptr_t cause, uintptr_t epc);

void __attribute__((weak))
handle_irq(uintptr_t cause, uintptr_t epc)
{

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Woverride-init"
#endif
	/* clang-format off */
	static void(* const irq_table[])(
		uintptr_t cause,
		uintptr_t epc) = {
		[0 ... 14]    = handle_irq_dummy,
		[IRQ_M_SOFT]  = handle_irq_m_soft,
		[IRQ_M_TIMER] = handle_irq_m_timer,
		[IRQ_M_EXT]   = handle_irq_m_ext,
	};
	/* clang-format on */
	
#if defined(__GNUC__)
#pragma GCC diagnostic warning "-Woverride-init"
#endif
	
    irq_table[cause & CAUSE_MACHINE_IRQ_REASON_MASK](cause, epc);
}
