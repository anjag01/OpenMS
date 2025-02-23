// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Author: Timo Sachsenberg, Mohammed Alhigaylan $
// $Maintainer: Timo Sachsenberg $
// -------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

#include <OpenMS/KERNEL/MSSpectrum.h>
#include <OpenMS/DATASTRUCTURES/Param.h>

namespace OpenMS
{
  /**
    @brief Peak picking algorithm for ion mobility data

  @ingroup PeakPicking
      */
    class OPENMS_DLLAPI PeakPickerIM
  {
  public:
    /// Default constructor initializing parameters with default values.
    PeakPickerIM();

    /// Destructor.
    virtual ~PeakPickerIM();

    /// Picks ion mobility traces from the given spectrum.
    void pickIMTraces(MSSpectrum& spectrum);

    /// Sets the parameters for peak picking.
    void setParameters(const Param& param);

    /// Gets the current parameters.
    Param getParameters() const;

  protected:
    /// Updates internal member variables when parameters are changed.
    void updateMembers_();

    /// Returns the default parameters.
    Param getDefaultParameters() const;

    /// Stores the parameters for peak picking.
    Param parameters_;
  };

} // namespace OpenMS