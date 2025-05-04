// Copyright (c) 2002-present, OpenMS Inc. -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Marc Sturm, Chris Bielow, Hannes Roest $
// --------------------------------------------------------------------------

#include <OpenMS/FORMAT/MzMLFile.h>

#include <OpenMS/FORMAT/HANDLERS/MzMLHandler.h>
#include <OpenMS/FORMAT/HANDLERS/XMLHandler.h>
#include <OpenMS/FORMAT/CVMappingFile.h>
#include <OpenMS/FORMAT/FileHandler.h>
#include <OpenMS/FORMAT/VALIDATORS/XMLValidator.h>
#include <OpenMS/FORMAT/VALIDATORS/MzMLValidator.h>
#include <OpenMS/FORMAT/TextFile.h>
#include <OpenMS/FORMAT/DATAACCESS/MSDataTransformingConsumer.h>
#include <OpenMS/FORMAT/HANDLERS/IndexedMzMLDecoder.h>
#include <OpenMS/SYSTEM/File.h>
#include <fstream>
#include <sstream>

namespace OpenMS
{

  MzMLFile::MzMLFile() :
    XMLFile("/SCHEMAS/mzML_1_10.xsd", "1.1.0"),
    indexed_schema_location_("/SCHEMAS/mzML_idx_1_10.xsd")
  {
  }

  MzMLFile::~MzMLFile() = default;

  PeakFileOptions& MzMLFile::getOptions()
  {
    return options_;
  }

  const PeakFileOptions& MzMLFile::getOptions() const
  {
    return options_;
  }

  void MzMLFile::setOptions(const PeakFileOptions& options)
  {
    options_ = options;
  }

  bool MzMLFile::hasIndex(const String& filename)
  {
    const std::streampos NOT_FOUND {-1};
    return NOT_FOUND != IndexedMzMLDecoder().findIndexListOffset(filename);
  }

  // reimplemented in order to handle index MzML
  bool MzMLFile::isValid(const String& filename, std::ostream& os)
  {
    //determine if this is indexed mzML or not
    bool indexed = false;
    TextFile file(filename, true, 4);
    String s;
    s.concatenate(file.begin(), file.end());
    if (s.hasSubstring("<indexedmzML"))
    {
      indexed = true;
    }
    // find the corresponding schema
    String current_location;
    if (indexed)
    {
      current_location = File::find(indexed_schema_location_);
    }
    else
    {
      current_location = File::find(schema_location_);
    }

    return XMLValidator().isValid(filename, current_location, os);
  }

  bool MzMLFile::isSemanticallyValid(const String& filename, StringList& errors, StringList& warnings)
  {
    // load mapping
    CVMappings mapping;
    CVMappingFile().load(File::find("/MAPPING/ms-mapping.xml"), mapping);

    // validate
    Internal::MzMLValidator v(mapping, ControlledVocabulary::getPSIMSCV());
    bool result = v.validate(filename, errors, warnings);

    return result;
  }

  void MzMLFile::loadSize(const String& filename, Size& scount, Size& ccount)
  {
    PeakMap dummy;
    Internal::MzMLHandler handler(dummy, filename, getVersion(), *this);
    handler.setOptions(options_);
    if (options_.hasFilters())
    {
      handler.setLoadDetail(Internal::XMLHandler::LD_COUNTS_WITHOPTIONS);
    }
    else
    { // no filters where specified. Just take the 'counts' attributes from the mzML file and end parsing
      handler.setLoadDetail(Internal::XMLHandler::LD_RAWCOUNTS);
    }
    
    safeParse_(filename, &handler);
    handler.getCounts(scount, ccount);
  }

  void MzMLFile::safeParse_(const String& filename, Internal::XMLHandler* handler)
  {
    try
    {
      parse_(filename, handler);
    }
    catch (Exception::BaseException& e)
    {
      String expr;
      expr += e.getFile();
      expr += "@";
      expr += e.getLine();
      expr += "-";
      expr += e.getFunction();
      throw Exception::ParseError(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION, expr, String("- due to that error of type ") + e.getName());
    }
  }

  void MzMLFile::loadBuffer(const std::string& buffer, PeakMap& map)
  {
    map.reset();

    Internal::MzMLHandler handler(map, "memory", getVersion(), *this);
    handler.setOptions(options_);
    parseBuffer_(buffer, &handler);
  }

  void MzMLFile::load(const String& filename, PeakMap& map)
  {
    map.reset();

    //set DocumentIdentifier
    map.setLoadedFileType(filename);
    map.setLoadedFilePath(filename);

    Internal::MzMLHandler handler(map, filename, getVersion(), *this);
    handler.setOptions(options_);
    safeParse_(filename, &handler);
  }


  void MzMLFile::store(const String& filename, const PeakMap& map) const
  {
    Internal::MzMLHandler handler(map, filename, getVersion(), *this);
    handler.setOptions(options_);
    save_(filename, &handler);
  }
  
  


  void MzMLFile::storeBuffer(std::string& output, const PeakMap& map) const
  {
    // First pass - write everything to calculate the offset
    Int64 xml_size = 0;
    std::string mzml_part;
    std::string indexedmzml_part;
    
    {
      // First write just the mzML part to a string
      std::stringstream mzml_stream;
      mzml_stream.precision(writtenDigits(double()));
      
      Internal::MzMLHandler handler(map, "dummy", getVersion(), *this);
      handler.setOptions(options_);
      handler.writeTo(mzml_stream);
      
      mzml_part = mzml_stream.str();
      xml_size = mzml_part.size();
    }
    
    // Calculate the position where indexList will start
    // Remove the closing indexedmzML tag which we'll rewrite
    size_t pos = mzml_part.find("</indexedmzML>");
    if (pos != std::string::npos)
    {
      mzml_part = mzml_part.substr(0, pos);
      
      // Find where the indexList starts
      size_t index_list_pos = mzml_part.find("<indexList");
      if (index_list_pos != std::string::npos)
      {
        // Extract the index content
        std::string index_content = mzml_part.substr(index_list_pos);
        
        // Calculate the offset - it's where </mzML> ends and <indexList> starts
        size_t mzml_end_pos = mzml_part.find("</mzML>");
        if (mzml_end_pos != std::string::npos)
        {
          Int64 offset = mzml_end_pos + 8; // "</mzML>\n" length
          
          // Rebuild the index section with the correct offset
          std::stringstream final_stream;
          final_stream << mzml_part.substr(0, index_list_pos);
          
          // Extract and rewrite index content up to indexListOffset
          size_t offset_pos = index_content.find("<indexListOffset>");
          if (offset_pos != std::string::npos)
          {
            final_stream << index_content.substr(0, offset_pos);
            final_stream << "<indexListOffset>" << offset << "</indexListOffset>\n";
            
            // Extract the rest after indexListOffset closing tag
            size_t after_offset = index_content.find("</indexListOffset>");
            if (after_offset != std::string::npos)
            {
              final_stream << index_content.substr(after_offset + 17); // +17 for "</indexListOffset>"
            }
          }
          else
          {
            // Just use the index content as is if we can't find the offset tag
            final_stream << index_content;
          }
          
          final_stream << "</indexedmzML>";
          output = final_stream.str();
          
          // Debug: Write output to a file for inspection
          std::ofstream debug_out("debug_mzml_output.xml");
          debug_out << output;
          debug_out.close();
          
          return;
        }
      }
    }
    
    // Fallback: If we couldn't parse and fix the offsets, return the original output
    output = mzml_part;
    
    // Debug: Write output to a file for inspection
    std::ofstream debug_out("debug_mzml_output.xml");
    debug_out << output;
    debug_out.close();
  }



  void MzMLFile::transform(const String& filename_in, Interfaces::IMSDataConsumer* consumer, bool skip_full_count, bool skip_first_pass)
  {
    // First pass through the file -> get the meta-data and hand it to the consumer
    if (!skip_first_pass) transformFirstPass_(filename_in, consumer, skip_full_count);

    // Second pass through the data, now read the spectra!
    {
      PeakMap dummy;
      Internal::MzMLHandler handler(dummy, filename_in, getVersion(), *this);
      handler.setOptions(options_);
      handler.setMSDataConsumer(consumer);
      safeParse_(filename_in, &handler);
    }
  }

  void MzMLFile::transform(const String& filename_in, Interfaces::IMSDataConsumer* consumer, PeakMap& map, bool skip_full_count, bool skip_first_pass)
  {
    // First pass through the file -> get the meta-data and hand it to the consumer
    if (!skip_first_pass)
    {
      transformFirstPass_(filename_in, consumer, skip_full_count);
    }
    // Second pass through the data, now read the spectra!
    {
      PeakFileOptions tmp_options(options_);
      Internal::MzMLHandler handler(map, filename_in, getVersion(), *this);
      tmp_options.setAlwaysAppendData(true);
      handler.setOptions(tmp_options);
      handler.setMSDataConsumer(consumer);

      safeParse_(filename_in, &handler);
    }
  }

  void MzMLFile::transformFirstPass_(const String& filename_in, Interfaces::IMSDataConsumer* consumer, bool skip_full_count)
  {
    // Create temporary objects and counters
    PeakFileOptions tmp_options(options_);
    Size scount = 0, ccount = 0;
    PeakMap experimental_settings;
    Internal::MzMLHandler handler(experimental_settings, filename_in, getVersion(), *this);

    // set temporary options for handler
    tmp_options.setMetadataOnly( skip_full_count );
    handler.setOptions(tmp_options);
    handler.setLoadDetail(Internal::XMLHandler::LD_RAWCOUNTS);

    safeParse_(filename_in, &handler);

    // After parsing, collect information
    handler.getCounts(scount, ccount);
    consumer->setExpectedSize(scount, ccount);
    consumer->setExperimentalSettings(experimental_settings);
  }

  std::map<UInt, MzMLFile::SpecInfo> MzMLFile::getCentroidInfo(const String& filename, const Size first_n_spectra_only)
  {
    bool oldoption = options_.getFillData();
    options_.setFillData(true); // we want the data as well (to allow estimation from data if metadata is missing)
    std::map<UInt, SpecInfo> ret;
    Size first_n_spectra_only_remaining = first_n_spectra_only;
    auto f = [&ret, &first_n_spectra_only_remaining](const MSSpectrum& s)
    {
        UInt lvl = s.getMSLevel();
        switch (s.getType(true))
        {
          case (MSSpectrum::SpectrumType::CENTROID):
            ++ret[lvl].count_centroided;
            --first_n_spectra_only_remaining;
            break;
          case (MSSpectrum::SpectrumType::PROFILE):
            ++ret[lvl].count_profile;
            --first_n_spectra_only_remaining;
            break;
          case (MSSpectrum::SpectrumType::UNKNOWN):  // this can only happen for spectra with very few peaks (or completely empty spectra)
            ++ret[lvl].count_unknown;
            break;
          default:
            // make sure we did not forget a case
            throw Exception::NotImplemented(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION);
        }

        // stop parsing after 10 or so spectra
        if (first_n_spectra_only_remaining == 0)
        {
          throw Internal::XMLHandler::EndParsingSoftly(__FILE__, __LINE__, OPENMS_PRETTY_FUNCTION);
        }
    };
    MSDataTransformingConsumer c;
    c.setSpectraProcessingFunc(f);
    transform(filename, &c, true, true); // no first pass

    // restore old state
    options_.setFillData(oldoption);

    return ret;
  }

} // namespace OpenMS
