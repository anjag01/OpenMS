// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Author: Timo Sachsenberg $
// $Maintainer: Timo Sachsenberg $
// -------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

#include <OpenMS/KERNEL/MSSpectrum.h>

namespace OpenMS
{
  /**
    @brief Peak picking algorithm for ion mobility data
    
    @ingroup PeakPicking
  */
  class OPENMS_DLLAPI PeakPickerIM
  {
    public:
        static void pickIMTraces(MSSpectrum& spectrum);
  };

} // namespace OpenMS