// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Author: Timo Sachsenberg $
// $Maintainer: Timo Sachsenberg $
// -------------------------------------------------------------------------------------------------------------------------------------------

#include <OpenMS/PROCESSING/CENTROIDING/PeakPickerIM.h>

namespace OpenMS
{
    void PeakPickerIM::pickIMTraces(MSSpectrum& spectrum)
    {        
        // determine IM format of spectrum
        IMFormat format = IMTypes::determineIMFormat(spectrum);
        switch (format)
        {
            case IMFormat::NONE:
                return; // no IM data
            case IMFormat::CENTROIDED:
                return; // already centroided
            case IMFormat::CONCATENATED:
                // TODO call peak picking algorithm for concatenated IM data

                // set format to centroided
                spectrum.setIMFormat(IMFormat::CENTROIDED);
                break;
            case IMFormat::UNKNOWN:
                throw Exception::InvalidValue(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION, "IMFormat set to UNKNOWN after deterineIMFormat. This should never happen. Please contact the developers.", String(NamesOfIMFormat[(size_t)format]));
            default:
                throw Exception::InvalidValue(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION, "Unknown IMFormat", String(NamesOfIMFormat[(size_t)format]));
        }
    }

} // namespace OpenMS