#include <fstream>
#include <sstream>
#include <string>
#include <cstddef>
#include <iostream>
#include <vector>
#include "utility.hpp"
#include "schedule.hpp"

using namespace std;
/*******************GENERAL TO DOS*******************************
 * TODO(baruch): Handle adding leading 0 to times that are under 4 wide 850 -> 0850
 * TODO(baruch): BUG in shortest route train time only code. Does not invalidate impossible routes for which departure time is during current train ride.
 * for precomputation, I need to finalize the new graph structure for the layover version and use that structure in the no-layover route solution also.
 * can't just use stations as vertex, must use departures, otherwise there is no way to precompute if the train routes are valid when a departure happens during
 * a ride.
 * 
 * **************************************************************/


int main(int argc, char** argv)
{
    std::ifstream stationFile;
    std::stringstream stationData;
    std::ifstream trainFile;
    std::stringstream trainData;

    if(argc < 3)
    {
        std::cout << "useage: ./sched.out <stations.dat> <trains.dat>\n";
        return 0;
    }

    // Get file data into a string so schedule can be constructed
    stationFile.open(argv[1]);
    stationData << stationFile.rdbuf();
    stationFile.close();

    stationFile.open(argv[2]);
    trainData << stationFile.rdbuf();
    trainFile.close();

    Schedule trainSchedule(stationData.str() , trainData.str());

    Utility::PrintMainMenu();

    bool quit = false;

    while(!quit)
    {
        std::cout << "Enter choice: ";

        int choice = -1;
        std:: cin >> choice;
        if(std::cin.fail())
        {
            choice = -1;
            Utility::ClearInStream();
        }

        switch (choice)
        {
            case 1:
                trainSchedule.PrintCompleteSchedule();
                break;
            case 2:
                trainSchedule.PrintStationSchedule();
                break;
            case 3:
                trainSchedule.LookUpStationId();
                break;
            case 4:
                trainSchedule.LookUpStationName();
                break;
            case 5:
                trainSchedule.GetRoute();
                break;
            case 6:
                trainSchedule.GetDirectRoute();
                break;
            case 7:
                trainSchedule.ShortestTripLengthRideTime();
                break;
            case 8:
                trainSchedule.ShortestTripLengthWithLayover();
                break;
            case 9:
                break;
            case 0:
                quit = true;
                std::cout << "Exiting...\n";
                break;
            default:
                Utility::PrintMainMenu();
                std::cout <<"Invalid choice (enter number 0-9).\n";
                break;    
        }
    }

}