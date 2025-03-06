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
#include <OpenMS/DATASTRUCTURES/Param.h>
#include <OpenMS/PROCESSING/RESAMPLING/LinearResamplerAlign.h>
#include <iostream>

namespace OpenMS
{
    double PeakPickerIM::computeOptimalSamplingRate(const MSSpectrum& spectrum)
    {
      if (spectrum.size() < 2)
      {
        std::cerr << "Warning: Spectrum has too few points for resampling. Using fixed sampling rate of 0.001 1/k" << std::endl;
        return 0.001; // Default fallback value
      }

      std::vector<double> mz_diffs;
      mz_diffs.reserve(spectrum.size() - 1);

      for (size_t i = 1; i < spectrum.size(); ++i)
      {
        mz_diffs.push_back(spectrum[i].getMZ() - spectrum[i - 1].getMZ());
      }

      // A mobilogram may have two peaks spaced far apart but in the 'm/z' array,
      // the peaks are adjacent to each other and will result in a big mz_dff.
      // Take the 75% percentile to remove large m/z gaps.
      std::vector<double> filtered_mz_diffs = mz_diffs;
      std::sort(filtered_mz_diffs.begin(), filtered_mz_diffs.end());
      double threshold = filtered_mz_diffs[filtered_mz_diffs.size() * 0.75];
      std::cerr << "75% percentile of ion mobility difference is determined to be... " << threshold << std::endl;

      std::vector<double> small_mz_diffs;
      for (double diff : mz_diffs)
      {
        if (diff <= threshold)
        {
          small_mz_diffs.push_back(diff);
        }
      }

      if (small_mz_diffs.empty())
      {
        std::cerr << "Warning: No valid small m/z differences found. Using default sampling rate. Using fixed sampling rate of 0.001 1/k" << std::endl;
        return 0.001; // Default fallback value
      }

      // Step 2: Compute mode (most common spacing)
      std::map<double, int> freq_map;
      for (double diff : small_mz_diffs)
      {
        freq_map[diff]++;
      }

      double mode_sampling_rate = small_mz_diffs.front(); // Default to first value
      int max_count = 0;
      for (const auto& [diff, count] : freq_map)
      {
        if (count > max_count)
        {
          mode_sampling_rate = diff;
          max_count = count;
        }
      }

      // Compute final sampling rate
      double sampling_rate = mode_sampling_rate;
      std::cout << "Computed sampling rate: " << sampling_rate << std::endl;

      return sampling_rate;
    }

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
        // Debugging: Print the input spectrum size
        std::cout << "Processing spectrum with " << spectrum.size() << " peaks." << std::endl;

        /*
        // IM format determination (Temporarily commented out)
        IMFormat format = IMTypes::determineIMFormat(spectrum);
        switch (format)
        {
            case IMFormat::NONE:
                return; // no IM data
            case IMFormat::CENTROIDED:
                return; // already centroided
            case IMFormat::UNKNOWN:
                throw Exception::InvalidValue(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION,
                    "IMFormat set to UNKNOWN after determineIMFormat. This should never happen.",
                    String(NamesOfIMFormat[(size_t)format]));
        }
        */

        // Initialize resampling
        double sampling_rate = computeOptimalSamplingRate(spectrum) * 4;
        std::cout << "Using sampling rate: " << sampling_rate << std::endl;

        // Set up custom parameters for the resampler
        bool ppm = false;
        Param resampler_param;
        resampler_param.setValue("spacing", sampling_rate, "Spacing of the resampled output peaks.");
        resampler_param.setValue("ppm", ppm ? "true" : "false", "Whether spacing is in ppm or Th");

        LinearResamplerAlign lin_resampler;
        lin_resampler.setParameters(resampler_param);

        lin_resampler.raster(spectrum);
        std::cout << "Size of resampled spectrum: " << spectrum.size() << std::endl;

        // Print resampled peaks for debugging
        for (const auto& peak : spectrum)
        {
            std::cout << "m/z: " << peak.getMZ()
                      << ", intensity: " << peak.getIntensity() << std::endl;
        }

        // Apply PeakPickerHiRes
        PeakPickerHiRes picker;
        picker.setParameters(parameters_);
        MSSpectrum picked_spectrum;

        picker.pick(spectrum, picked_spectrum);
        std::cout << "Size of picked spectrum: " << picked_spectrum.size() << std::endl;

        // Replace original spectrum with processed version
        spectrum = picked_spectrum;

        // Temporarily skipping IM format setting since we commented out the check
        // spectrum.setIMFormat(IMFormat::CENTROIDED);
    }


} // namespace OpenMS