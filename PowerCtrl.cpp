//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

typedef int IRQn_Type;
#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    1
#include <Arduino.h>
#include <libmaple/libmaple_types.h>
#include <libmaple/usbF4/VCP/core_cm4.h>
#include <libmaple/usbF4/VCP/core_cmInstr.h>
#include <libmaple/pwr.h>
#include "PowerCtrl.h"

PowerCtrl& PowerCtrl::GetInstance() {
  static PowerCtrl instance;
  // standby mode check
  if (PWR_BASE->CSR & (1UL << PWR_CSR_SBF)) {
    SerialUSB.println("wake up from standby mode");
    PWR_BASE->CSR |= (1UL << PWR_CR_CSBF);
  }
  return instance;
}

// UWF繧偵け繝ｪ繧｢縺吶ｋ
void PowerCtrl::ClearWUF() {
  // clear WUF bit
  PWR_BASE->CR |= (1UL << PWR_CR_CWUF); 
}

// Standby繝｢繝ｼ繝峨∈驕ｷ遘ｻ
void PowerCtrl::EnterStandbyMode() {

  // set PDDS bit
  PWR_BASE->CR |= (1UL << PWR_CR_PDDS);

  // clear WUF bit
  ClearWUF();

  // WUP縺後け繝ｪ繧｢縺輔ｌ繧九∪縺ｧ蠕�縺､
  while(PWR_BASE->CSR & (1UL << PWR_CSR_WUF)) {
  }
  
  // set sleep deep bit
  SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk);

  // 蜑ｲ繧願ｾｼ縺ｿ蠕�縺｡�ｼ�Standby繝｢繝ｼ繝会ｼ�
  __WFI();
}

