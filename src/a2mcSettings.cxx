///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcSettings.h"

a2mcSettings::a2mcSettings() :
    status(true),
    gen_mode(0),
    inn_enviro(0),
    sil_det(0),
    out_enviro(0),
    store_tracks(false),
    tracks_lim(0),
    verbose(0)
{
    init(std::string(INI_INSTALL_PATH) + "/a2MC.ini");
}

a2mcSettings::~a2mcSettings()
{}

void a2mcSettings::init(std::string conf_filename)
{
    ///< This method read the configuration file and fill the corresponding flags 
    std::string line, dummy;
    std::ifstream myfile (conf_filename.c_str());
    if (myfile.is_open()) {
        size_t found;
        while ( getline (myfile,line) ) {
        	///< Skipping comment lines
        	if(line.find("#")!=std::string::npos) continue;
            ///< Looking for gen_type
            found = line.find("gen_type");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> gen_type;
            }
            ///< Looking for gen_mode
            found = line.find("gen_mode");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> gen_mode;
            }
            ///< Looking for the inner environmental flag
            found = line.find("inn_enviro");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> inn_enviro;
            }
            ///< Looking for the silicon detector
            found = line.find("sil_det");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> sil_det;
            }
            ///< Looking for the outer environmental flag
            found = line.find("out_enviro");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> out_enviro;
            }
            ///< Looking for the magnetic field 
            found = line.find("mag_field");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> mag_field;
            }
            ///< Looking for store_tracks
            found = line.find("store_tracks");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> store_tracks;
            }
            ///< Looking for tracks_lim
            found = line.find("tracks_lim");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> tracks_lim;
            }
            ///< Looking for verbose
            found = line.find("verbose");
            if(found!=std::string::npos) {
                std::istringstream iss(line);
                iss >> dummy >> verbose;
            }
        }
        myfile.close();
    }  else {
        status = false;
        std::cout << "a2mcSettings::init -> File " <<  conf_filename.c_str() << " doesn't exist - please get it from the git repository (input subdir)" << std::endl; 
    }
}

void a2mcSettings::Print() 
{
    std::cout << " status "         << status       << std::endl;
    std::cout << " gen_mode "       << gen_mode     << std::endl;
    std::cout << " inn_enviro "     << inn_enviro   << std::endl;
    std::cout << " sil_det "        << sil_det      << std::endl;
    std::cout << " out_enviro "     << out_enviro   << std::endl;
    std::cout << " store_tracks "   << store_tracks << std::endl;
    std::cout << " tracks_lim "     << tracks_lim   << std::endl;
    std::cout << " verbose "        << verbose      << std::endl;
}
