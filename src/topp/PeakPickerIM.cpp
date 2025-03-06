// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Author: Mohammed Alhigaylan $
// $Maintainer: Timo Sachsenberg $
// -------------------------------------------------------------------------------------------------------------------------------------------

#include <OpenMS/CONCEPT/LogStream.h>
#include <OpenMS/FORMAT/FileHandler.h>
#include <OpenMS/FORMAT/MzMLFile.h>
#include <OpenMS/KERNEL/MSExperiment.h>
#include <OpenMS/APPLICATIONS/TOPPBase.h>
#include <OpenMS/FORMAT/DATAACCESS/MSDataWritingConsumer.h>
#include <OpenMS/PROCESSING/CENTROIDING/PeakPickerIM.h>

using namespace OpenMS;
using namespace std;

class TOPPPeakPickerIM : public TOPPBase
{
public:
  TOPPPeakPickerIM() : TOPPBase("PeakPickerIM", "Applies PeakPickerIM to an mzML file", false) {}

protected:
  void registerOptionsAndFlags_() override
  {
    registerInputFile_("in", "<file>", "", "Input mzML file");
    setValidFormats_("in", ListUtils::create<String>("mzML"));

    registerOutputFile_("out", "<file>", "", "Output mzML file");
    setValidFormats_("out", ListUtils::create<String>("mzML"));
  }

  ExitCodes main_(int, const char**) override
  {
    // Get input and output file paths
    String input_file = getStringOption_("in");
    String output_file = getStringOption_("out");

    // Load input mzML file
    PeakMap exp;
    MzMLFile mzml;
    mzml.load(input_file, exp);

    // Process each spectrum with PeakPickerIM
    PeakPickerIM picker;
    PeakMap processed_exp;
    for (MSSpectrum& spectrum : exp)
    {
      std::cout << "Processing spectrum with " << spectrum.size() << " peaks." << std::endl;
      picker.pickIMTraces(spectrum);
      processed_exp.addSpectrum(spectrum);
      std::cout << "Processed spectrum has " << spectrum.size() << " peaks." << std::endl;
    }
//    processed_exp.setExperimentalSettings(exp);
    // Save output mzML file
    mzml.store(output_file, processed_exp);

    return EXECUTION_OK;
  }
};

int main(int argc, const char** argv)
{
  TOPPPeakPickerIM tool;
  return tool.main(argc, argv);
}

