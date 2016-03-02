#include "xAODRootAccess/Init.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoopGrid/PrunDriver.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/DiskListEOS.h"
#include <TSystem.h>

#include "MultijetBalanceAlgo/MultijetAlgorithim.h"

#include "xAODAnaHelpers/BasicEventSelection.h"
#include "xAODAnaHelpers/Writer.h"

#include <string>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>

#include "TEnv.h"
#include "TString.h"
#include "TSystem.h"

using namespace std;

int main( int argc, char* argv[] ) {

  // Take the submit directory from the input if provided:
  std::string samplePath = ".";
  std::string inputTag = "";
  std::string outputTag = "";
  std::string submitDir = "submitDir";
  std::string configName = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo_EM.config";
  std::string productionName = "";
  bool f_oneJobPerFile = false;

  /////////// Retrieve runDijetResonance's arguments //////////////////////////
  std::vector< std::string> options;
  for(int ii=1; ii < argc; ++ii){
    options.push_back( argv[ii] );
  }

  if (argc > 1 && options.at(0).compare("-h") == 0) {
    std::cout << std::endl
         << " runDijetResonance : DijetResonance job submission" << std::endl
         << std::endl
         << " Optional arguments:" << std::endl
         << "  -h                Prints this menu" << std::endl
         << "  --file            Path to a folder, root file, or text file" << std::endl
         << "  --inputTag        A wildcarded file name to run on" << std::endl
         << "  --tag             Version string to be appended to job name" << std::endl
         << "  --submitDir       Name of output directory" << std::endl
         << "  --config          Path to config file" << std::endl
         << "  --production      Group name for the production role" << std::endl
         << "  --oneJobPerFile      Only 1 job per file" << std::endl
         << std::endl;
    exit(1);
  }

  int iArg = 0;
  while(iArg < argc-1) {
    if (options.at(iArg).compare("-h") == 0) {
       // Ignore if not first argument
       ++iArg;
    } else if (options.at(iArg).compare("--file") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --file should be followed by a file or folder" << std::endl;
         return 1;
       } else {
         samplePath = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--oneJobPerFile") == 0) {
      ++iArg;
      f_oneJobPerFile = true;
    } else if (options.at(iArg).compare("--inputTag") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --inputTag is a wildcarded file name to run on" << std::endl;
         return 1;
       } else {
         inputTag = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--tag") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --tag should be followed by a job version string" << std::endl;
         return 1;
       } else {
         outputTag = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--submitDir") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --submitDir should be followed by a folder name" << std::endl;
         return 1;
       } else {
         submitDir = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--config") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --config should be followed by a config file" << std::endl;
         return 1;
       } else {
         configName = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--production") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc-1 || tmpChar == '-' ) {
         std::cout << " --production should be followed by the production role (i.e. phys-exotics)" << std::endl;
         return 1;
       } else {
         productionName = options.at(iArg+1);
         iArg += 2;
       }
    }else{
      std::cout << "Couldn't understand argument " << options.at(iArg) << std::endl;
      return 1;
    }
  }//while arguments


  bool f_grid = false;
  bool f_lxbatch = false;
  bool f_production = false;
  if (productionName.size() > 0)
    f_production = true;


  xAOD::Init().ignore();

  //////////// Config file information //////////////
  TEnv* config = new TEnv( gSystem->ExpandPathName(configName.c_str()) );
  std::string BasicEventSelectionConfig = config->GetValue("BasicEventSelectionConfig", "$ROOTCOREBIN/data/DijetResonanceAlgo/baseEvent.config");
  // correct config name to include $ROOTCOREBIN
  configName.erase(0, configName.find_last_of("/")+1);
  configName = "$ROOTCOREBIN/data/MultijetBalanceAlgo/"+configName;


  //////////// Construct the samples to run on //////////////////////
  SH::SampleHandler sh;
  std::string containerName;
  std::string userName = getlogin();
  if (f_production)
    userName = productionName;
  std::vector< std::string> outputContainerNames; //for grid only

  //Check if input is a directory or a file
  struct stat buf;
  stat(samplePath.c_str(), &buf);
  if( samplePath.substr(0, 4).find("eos") != std::string::npos){
    SH::DiskListEOS list(samplePath.c_str());
    if (inputTag.size() > 0){
      SH::scanDir (sh, list, inputTag); //Run on all files within dir containing inputTag
    }else{
      SH::scanDir (sh, list); //Run on all files within dir
    }
    std::cout << "Running on EOS directory " << samplePath << std::endl;
  }else if( S_ISDIR(buf.st_mode) ){ //if it is a local directory
    SH::DiskListLocal list (samplePath);
    if (inputTag.size() > 0){
      SH::scanDir (sh, list, inputTag); //Run on all files within dir containing inputTag
    }else{
      SH::scanDir (sh, list); //Run on all files within dir
    }
    std::cout << "Running Locally on directory  " << samplePath << std::endl;

  } else {  //if it is a file
    if( samplePath.substr( samplePath.size()-4 ).find(".txt") != std::string::npos){ //It is a text file of samples
      if( samplePath.find("grid") != std::string::npos ) //It is samples for the grid
        f_grid = true;

      std::ifstream inFile( samplePath );
      while(std::getline(inFile, containerName) ){
        if (containerName.size() > 1 && containerName.find("#") != 0 ){
          cout << "Adding container " << containerName << endl;
          if(f_grid){
            SH::scanDQ2( sh, containerName);
            //Add output container name to file of containers
            //follows grid format: "user."+userName+".%in:name[1]%.%in:name[2]%.%in:name[3]%"+outputTag
            int namePosition = 0;
            namePosition = containerName.find_first_of(".", namePosition)+1;
            namePosition = containerName.find_first_of(".", namePosition)+1;
            namePosition = containerName.find_first_of(".", namePosition)+1;
            outputContainerNames.push_back( ("user."+userName+"."+containerName.substr(0, namePosition)+outputTag+"/") );
          }else{
            //Get full path of file
            char fullPath[300];
            realpath( containerName.c_str(), fullPath );
            string thisPath = fullPath;
            //split into fileName and directory two levels above file
            string fileName = thisPath.substr(containerName.find_last_of("/")+1);
            thisPath = thisPath.substr(0, thisPath.find_last_of("/"));
            thisPath = thisPath.substr(0, thisPath.find_last_of("/"));
            cout << "path and filename are " << thisPath << " and " << fileName << endl;

            SH::DiskListLocal list (thisPath);
            //SH::SampleHandler sh_tmp;
            //SH::scanDir (sh_tmp, list);
            //sh.add( sh_tmp.findByName, ("*"+fileName).c_str() );
            SH::scanDir (sh, list, fileName); // specifying one particular file for testing
          }
        }
      }

    }else{ //It is a single file to run on

    //Get full path of file
    char fullPath[300];
    realpath( samplePath.c_str(), fullPath );
    string thisPath = fullPath;
    //split into fileName and directory two levels above file
    string fileName = thisPath.substr(thisPath.find_last_of("/")+1);
    thisPath = thisPath.substr(0, thisPath.find_last_of("/"));
    thisPath = thisPath.substr(0, thisPath.find_last_of("/"));

    cout << "path and file " << thisPath << " and " << fileName << endl;
    SH::DiskListLocal list (thisPath);
    SH::scanDir (sh, list, fileName); // specifying one particular file for testing

    }
  }//it's a file
//
  ///// Set output container name ///////
  std::string outputName;
  if( outputTag.compare("None") != 0)
    outputTag = "."+outputTag+"/";
  else
    outputTag = "/";

  if(f_grid){
    if (f_production)
      outputName = "group."+userName+".%in:name[1]%.%in:name[2]%.%in:name[3]%"+outputTag;
    else
      outputName = "user."+userName+".%in:name[1]%.%in:name[2]%.%in:name[3]%"+outputTag;
  }else
    outputName = "%in:name%"+outputTag;
cout << outputName << endl;

  // Set the name of the input TTree. It's always "CollectionTree"
  sh.setMetaString( "nc_tree", "CollectionTree" );
  sh.setMetaString("nc_grid_filter", "*");
  sh.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler( sh );

  // To automatically delete submitDir
  job.options()->setDouble(EL::Job::optRemoveSubmitDir, 1);

  // For Trigger
//  job.options()->setString( EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_branch );
  job.options()->setString( EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_class );

  //// basic event selection : GRL, event cleaning, NPV
  BasicEventSelection* baseEventSel = new BasicEventSelection();
  baseEventSel->setName( "baseEventSel" )->setConfig( BasicEventSelectionConfig.c_str() );

  // Add our analysis to the job:
  MultijetAlgorithim* multijetAlg = new MultijetAlgorithim( "MultijetAlgo", configName.c_str() );

  // ADD ALGOS TO JOB
  job.algsAdd( baseEventSel );
  job.algsAdd( multijetAlg );


  if(f_grid){
    EL::PrunDriver driver;
    driver.options()->setString("nc_outputSampleName", outputName);
    if( f_oneJobPerFile ){
      std::cout << "Setting code to 1 file per job " << std::endl;
      driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 1);
    }
    //driver.options()->setString("nc_grid_filter", "*.AOD.*");
    if (f_production)
      driver.options()->setString(EL::Job::optSubmitFlags, "--official");
    driver.options()->setString("nc_skipScout", "true");


    driver.options()->setString("showCmd", "true");

    //driver.options()->setString("nc_excludeSite", ???);
    //driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB
    driver.submitOnly(job, submitDir); //without monitoring
  }else if( f_lxbatch){
    std::cout << "Currently not implemented! " << std::endl;
  }else{
    // Run the job using the local/direct driver:
    EL::DirectDriver driver;
    driver.options()->setString("nc_outputSampleName", outputName);
    driver.submit( job, submitDir );
  }

  ///// Save list of ouput containers to the submission directory /////
  std::ofstream fileList((submitDir+"/outputContainers.txt"), std::ios_base::out);
  for( unsigned int iCont=0; iCont < outputContainerNames.size(); ++iCont){
    fileList << outputContainerNames.at(iCont)+"\n";
  }
  fileList.close();

  return 0;
}
