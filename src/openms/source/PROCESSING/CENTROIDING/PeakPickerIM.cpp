// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Author: Timo Sachsenberg, Mohammed Alhigaylan $
// $Maintainer: Timo Sachsenberg $
// -------------------------------------------------------------------------------------------------------------------------------------------

#include <OpenMS/PROCESSING/CENTROIDING/PeakPickerIM.h>
#include <OpenMS/PROCESSING/CENTROIDING/PeakPickerHiRes.h>
#include <OpenMS/KERNEL/MSSpectrum.h>
#include <OpenMS/CONCEPT/Exception.h>
#include <OpenMS/DATASTRUCTURES/Param.h>  // Updated include
#include <iostream>

namespace OpenMS
{
    // Constructor: initialize the parameters_ member with default parameters.
    PeakPickerIM::PeakPickerIM() :
        parameters_(getDefaultParameters())
    {
    }

    // Destructor
    PeakPickerIM::~PeakPickerIM()
    {
    }

    // Returns a Param object with the default settings for peak picking.
    Param PeakPickerIM::getDefaultParameters() const
    {
      Param p;
      p.setValue("signal_to_noise", 0.0, "Signal to noise threshold for peak picking");
      p.setValue("spacing_difference_gap", 0.0, "The extension of a peak is stopped if the spacing between two subsequent data points exceeds 'spacing_difference_gap * min_spacing'. 'min_spacing' is the smaller of the two spacings from the peak apex to its two neighboring points. '0' to disable the constraint. Not applicable to chromatograms.");
      p.setValue("spacing_difference", 0.0, "Maximum allowed difference between points during peak extension, in multiples of the minimal difference between the peak apex and its two neighboring points. If this difference is exceeded a missing point is assumed (see parameter 'missing'). A higher value implies a less stringent peak definition, since individual signals within the peak are allowed to be further apart. '0' to disable the constraint. Not applicable to chromatograms.");
      p.setValue("missing", 0, "Maximum number of missing points allowed when extending a peak to the left or to the right. A missing data point occurs if the spacing between two subsequent data points exceeds 'spacing_difference * min_spacing'. 'min_spacing' is the smaller of the two spacings from the peak apex to its two neighboring points. Not applicable to chromatograms.");
      p.setValue("signal_to_noise", 0.0, "Signal to noise threshold for peak picking");
      return p;
    }
    // Update internal members if any parameter changes require it.
    void PeakPickerIM::updateMembers_()
    {
      // For this example, no extra member variables need updating.
      // This function is a placeholder for potential future use.
    }

    // Sets the parameters and updates internal members accordingly.
    void PeakPickerIM::setParameters(const Param& param)
    {
      parameters_ = param;
      updateMembers_();
    }

    // Retrieves the current parameters.
    Param PeakPickerIM::getParameters() const
    {
      return parameters_;
    }

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
            case IMFormat::CONCATENATED: {
                // TODO call peak picking algorithm for concatenated IM data
                std::cout << "Processing concatenated IM data..." << std::endl;
                std::cout << "Size of input spectrum..." << spectrum.size() << std::endl;

                PeakPickerHiRes picker;
                // Forward the parameters from this object to the underlying picker
                picker.setParameters(parameters_);
                MSSpectrum picked_spectrum;

                picker.pick(spectrum, picked_spectrum);
                std::cout << "Size of picked_spectrum..." << picked_spectrum.size() << std::endl;

                spectrum = picked_spectrum;

                // set format to centroided
                spectrum.setIMFormat(IMFormat::CENTROIDED);
                break;
            }
            case IMFormat::UNKNOWN:
                throw Exception::InvalidValue(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION, "IMFormat set to UNKNOWN after determineIMFormat. This should never happen. Please contact the developers.", String(NamesOfIMFormat[(size_t)format]));
            default:
                throw Exception::InvalidValue(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION, "Unknown IMFormat", String(NamesOfIMFormat[(size_t)format]));
        }
    }

} // namespace OpenMS