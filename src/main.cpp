/**
 ** EEG Logger : Capture EEG info ONLINE from Emotiv and dump into plain ascii files.
 **
 ** You can use those files in MATLAB like this:

function output = loadepoceegrawbyfile(fullfile,dowemean)

	fid = fopen( fullfile );

	output_matrix = fscanf(fid, '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f', [22 inf]);

	fclose(fid);

	output_matrix = output_matrix';
	output = output_matrix(:,2:15);

	if (dowemean == 1)
		[n,m]=size(output);
		output=output - ones(n,1)*mean(output,1);
	end

end

 ** This program is a modification of the example5 that comes with the EPOC SDK library.
 **
 ** Rodrigo Ramele @ CiC Lab @ ITBA!
 **/


#include <iostream>
#include <fstream>
#include <conio.h>
#include <sstream>
#include <windows.h>
#include <map>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#pragma comment(lib, "lib/edk.lib")

#define	PATH	"C:\\Users\\User\\Desktop\\Data\\%s\\%s"


// This is the filter that we apply to EDK to tell him what field do we want to retrieve from the transmitted package from Emotiv.
// In this case, we are using a 22 field stream.  This must match the definition that we can use later from Matlab in the fscanf command.
EE_DataChannel_t targetChannelList[] = {
		ED_COUNTER,
		ED_AF3, ED_F7, ED_F3, ED_FC5, ED_T7, 
		ED_P7, ED_O1, ED_O2, ED_P8, ED_T8, 
		ED_FC6, ED_F4, ED_F8, ED_AF4, ED_GYROX, ED_GYROY, ED_TIMESTAMP, 
		ED_FUNC_ID, ED_FUNC_VALUE, ED_MARKER, ED_SYNC_SIGNAL
	};


const char header[] = "COUNTER,AF3,F7,F3, FC5, T7, P7, O1, O2,P8" 
                      ", T8, FC6, F4,F8, AF4,GYROX, GYROY, TIMESTAMP, "   
                      "FUNC_ID, FUNC_VALUE, MARKER, SYNC_SIGNAL,";

int  eeglogger(char *filename, char *subject, int duration);

int main(int argc, char **argv) {

	char *filename;
	char *fullfilename;
	char *subject;
	int duration = 10;

	fullfilename = new char[256];

	subject = new char[256];
	// defaulted subject
	strcpy( subject, "KarlNeuron");

	if (argc < 2) {
		filename = new char[256];
		strcpy(filename,"eeg.log");
		duration = 10;
	} else if (argc == 4)
	{
		filename = argv[1];
		strcpy( subject, argv[2]);
		duration = atoi(argv[3]);
	} else 
		throw std::exception("Please supply the log file name.\nUsage: Epoceeg [dat_filename without extension] [subject] [duration in seconds].");


	// You can call here the experiment_protocol_sample or something similar.

	return eeglogger(filename, subject, duration);
}

int experiment_protocol_sample(char *fullfilename, char *filename, char *subject) {
	std::cout << "S1: Relaxed eyes open." << std::endl;getchar();

	sprintf(fullfilename,"%s_S1.dat",filename);eeglogger(fullfilename,subject,30);
	
	std::cout << "S2: Relaxed eyes closed." << std::endl;getchar();
	
	sprintf(fullfilename,"%s_S2.dat",filename);eeglogger(fullfilename,subject,30);

	for(int i=1;i<=6;i++)
	{
		std::cout << "E" << i << ": Ready?" << std::endl;getchar();
		sprintf(fullfilename,"%s_E%d.dat",filename,i);eeglogger(fullfilename,subject,70);
	}

	return 1;
}

int  eeglogger(char *filename, char *subject, int duration)
{
	EmoEngineEventHandle eEvent			= EE_EmoEngineEventCreate();
	EmoStateHandle eState				= EE_EmoStateCreate();
	unsigned int userID					= 0;
	const unsigned short composerPort	= 1726;
	float secs							= 1;
	unsigned int datarate				= 0;
	bool readytocollect					= false;
	int option							= 0;
	int state							= 0;


	std::string input;
	char *fullfilename;

	int frequency						= 128;

	int tick							=0;

	try {

		fullfilename = new char[256];

		char temp[256];

		sprintf ( temp , PATH, subject,"");

		sprintf ( temp, "mkdir %s", temp);

		system( temp );

		std::cout << temp << std:enld;

		sprintf( fullfilename, PATH, subject, filename);

		/**
		std::cout << "===================================================================" << std::endl;
		std::cout << "Example to show how to log EEG Data from EmoEngine/EmoComposer."	   << std::endl;
		std::cout << "===================================================================" << std::endl;
		std::cout << "File:" << fullfilename << std::endl;
		std::cout << "Press '1' to start and connect to the EmoEngine                    " << std::endl;
		std::cout << "Press '2' to connect to the EmoComposer                            " << std::endl;
		std::cout << ">> ";

		std::getline(std::cin, input, '\n');
		option = atoi(input.c_str());
		**/

		option = 1;

		switch (option) {
			case 1:
			{
				if (EE_EngineConnect() != EDK_OK) {
					throw std::exception("Emotiv Engine start up failed.");
				}
				break;
			}
			case 2:
			{
				std::cout << "Target IP of EmoComposer? [127.0.0.1] ";
				std::getline(std::cin, input, '\n');

				if (input.empty()) {
					input = std::string("127.0.0.1");
				}

				if (EE_EngineRemoteConnect(input.c_str(), composerPort) != EDK_OK) {
					std::string errMsg = "Cannot connect to EmoComposer on [" + input + "]";
					throw std::exception(errMsg.c_str());
				}
				break;
			}
			default:
				throw std::exception("Invalid option...");
				break;
		}
		
		
		std::cout << "Profile" << std::endl;
		std::cout << "Subject:" << subject << std::endl;
		std::cout << "Experiment:" << filename << std::endl;
		std::cout << "Fullfilename:" << fullfilename << std::endl;
		std::cout << "Duration:" << duration << std::endl;

		std::cout << "Start receiving EEG Data! Press any key to stop logging...\n" << std::endl;
    	std::ofstream ofs(fullfilename,std::ios::trunc);
		//ofs << header << std::endl;
		
		DataHandle hData = EE_DataCreate();
		EE_DataSetBufferSizeInSec(secs);

		std::cout << "Buffer size in secs:" << secs << std::endl;

		while (/**!_kbhit() && */ tick < frequency * duration) {

			state = EE_EngineGetNextEvent(eEvent);

			if (state == EDK_OK) {

				EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent);
				EE_EmoEngineEventGetUserId(eEvent, &userID);

				// Log the EmoState if it has been updated
				if (eventType == EE_UserAdded) {
					std::cout << "User added";
					EE_DataAcquisitionEnable(userID,true);
					readytocollect = true;
				}
			}

			if (readytocollect) {
						
						EE_DataUpdateHandle(0, hData);

						unsigned int nSamplesTaken=0;
						EE_DataGetNumberOfSample(hData,&nSamplesTaken);
		
						std::cout << "[" <<(tick / frequency * 1.0)<< "]" << ":" << "Updated " << nSamplesTaken <<  ", Channels:" << sizeof(targetChannelList)/sizeof(EE_DataChannel_t) << std::endl;

						if (nSamplesTaken != 0) {

							double* data = new double[nSamplesTaken];
							for (int sampleIdx=0 ; sampleIdx<(int)nSamplesTaken ; ++ sampleIdx) {
								for (int i = 0 ; i<sizeof(targetChannelList)/sizeof(EE_DataChannel_t) ; i++) {

									EE_DataGet(hData, targetChannelList[i], data, nSamplesTaken);
									ofs << data[sampleIdx] << " ";
								}	
								ofs << std::endl;
								tick++;
							}
							delete[] data;
						}

			}

			Sleep(100);
		}

		ofs.close();
		EE_DataFree(hData);

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::cout << "Press any key to exit..." << std::endl;
		getchar();
	}

	EE_EngineDisconnect();
	EE_EmoStateFree(eState);
	EE_EmoEngineEventFree(eEvent);

	return 0;
}


