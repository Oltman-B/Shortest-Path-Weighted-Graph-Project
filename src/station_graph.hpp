#pragma once
#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include "station.hpp"
#include "departure.hpp"
#include "route.hpp"

class StationGraph{
    public:
        StationGraph(std::vector<std::vector<std::string>> const tripData, int stationsCount);
        ~StationGraph();
        bool DirectPathExists(int station1ID, int station2ID);
        bool PathExists(int startStationID, int targetStationID);        
        Station GetStationFromGraph(int stationID);
        Route GetShortestRoute(int departureStationID, int destinationStationID, bool includeLayovers);
        Station GetStationFromArrivalGraph(int stationID);
        int GetVertexCount();
    private:
        const int stationCount;
        //Departure graph is the standard graph used for operations concerned with going from a starting node to a
        // destination node, this is the majority of the functions in the program.
        std::vector<Station>* stationsGraphList;
        // Arrivals graph is an inverted version of the graph so that arrivals to a station can be
        // looked up easily. Only used for a few functions in the program.
        std::vector<Station>* stationArrivalsGraphList;
        // Used to calculate shortest route accounting for layovers.
        std::vector<Departure>* departureGraphList;
        std::vector<std::vector<int>>* shortestRouteWithLayoverSequenceTable;
        std::vector<std::vector<int>>* shortestRouteWithoutLayoverSequenceTable;
        void floyd_warshal_shortest_paths(bool includeLayovers);
        Route get_route(int departureKey, int destinationKey, std::vector<std::vector<int>>& routeLookUpTable);
        Route get_shortest_route(int departureID, int destinationID, std::vector<std::vector<int>>& routeLookUpTable);        
        void build_stations_graph(std::vector<std::vector<std::string>> tripData);
        void build_station_arrivals_graph(std::vector<std::vector<std::string>> tripData);
        void build_departures_graph(std::vector<std::vector<std::string>> tripData);
};

StationGraph::StationGraph(std::vector<std::vector<std::string>> const tripDataTable, int stationsCount) : stationCount(stationsCount)
{
    build_stations_graph(tripDataTable);
    build_station_arrivals_graph(tripDataTable);
    build_departures_graph(tripDataTable);
    floyd_warshal_shortest_paths(true);
    
    /*//DEBUG PRINTING
    std::cout << "\n*****DEPARTURE GRAPH TEST in floyd warshal shortest path include layovers function*****\n";
    for(int i = 0; i < departureGraphList->size(); i++)
    {
        Departure station = (*departureGraphList)[i];
        std::cout << "Vertex: " << station.GetLookUpKey() << ":\n";
        for(int j = 0; j < station.GetTripCount(); j++)
        {
            TripPlusLayover trip = station.GetTrip(j);
            std::cout << "  Destination ID: " << trip.destinationKey << " Trip Weight: " << trip.tripWeight << std::endl;
        }
    }
    //END DEBUG PRINTING*/
}

StationGraph::~StationGraph()
{
    if(stationsGraphList) delete stationsGraphList;
    if(stationArrivalsGraphList) delete stationArrivalsGraphList;
    if(departureGraphList) delete departureGraphList;
    if(shortestRouteWithLayoverSequenceTable) delete shortestRouteWithLayoverSequenceTable;
    if(shortestRouteWithoutLayoverSequenceTable) delete shortestRouteWithoutLayoverSequenceTable;
}

void StationGraph::build_stations_graph(std::vector<std::vector<std::string>> tripDataTable)
{
    // Use a temporary table to hold all trips so that they
    // can be passed into station constructor.
    // We don't want to allow the addition of new graph elements after construction because we are pre-computing shortest paths
    // so not making any public functions to add nodes or edges.
    stationsGraphList = new std::vector<Station>;

    std::vector<std::vector<Trip>> tempTripTable;
    for(int i = 0; i < stationCount; i++)
    {
        tempTripTable.push_back({});
    }

    //Add the trip data to tempTripTable array
    for(int i = 0; i < tripDataTable.size(); i++)
    {
        int startID = stoi(tripDataTable[i][0]) - 1;
        int destinationID = stoi(tripDataTable[i][1]);
        int arrivalTime = stoi(tripDataTable[i][3]);
        int departureTime = stoi(tripDataTable[i][2]);
        int travelTimeMins = (arrivalTime - departureTime);
        tempTripTable[startID].push_back({destinationID, departureTime, arrivalTime, travelTimeMins});
    }

    //Construct the stations and add trips to graph.
    for(int i = 0; i < stationCount; i++)
    {
        stationsGraphList->push_back({i + 1, tempTripTable[i]});
    }
}

void StationGraph::build_departures_graph(std::vector<std::vector<std::string>> tripDataTable)
{
    departureGraphList = new std::vector<Departure>;
    std::vector<std::pair<std::pair<int, int>, std::vector<TripPlusLayover>>> tempTripTable;

    for(int i = 0; i < (tripDataTable.size()); i++)
    {
        tempTripTable.push_back({});
    }

    for(int i = 0; i < tripDataTable.size(); i++)
    {        
        int destinationKey;
        int departureTime = stoi(tripDataTable[i][2]);
        int rideTimeToDestination = stoi(tripDataTable[i][3]) - stoi(tripDataTable[i][2]);
        int layoverAtDestination = 0; // this node marks end of trip, no layover added.
        int totalTripTime = rideTimeToDestination + layoverAtDestination;

        // Map trip target station id to the lookUpKey of the vertex.
        for(int keyIndx = 0; keyIndx < tripDataTable.size(); keyIndx++)
        {
            if(stoi(tripDataTable[keyIndx][0]) == stoi(tripDataTable[i][0]) && stoi(tripDataTable[keyIndx][2]) == stoi(tripDataTable[i][2]))
            {                
                destinationKey = keyIndx;
            }
        }

        for(int k = 0; k < tripDataTable.size(); k++)
        {
            //If the target edge departure time and departure station match, this is the correct insertion point, add edge to adjacency list.
            if(stoi(tripDataTable[k][0]) == stoi(tripDataTable[i][0]) && stoi(tripDataTable[k][2]) == stoi(tripDataTable[i][2]))
            {
                tempTripTable[k].first.first = departureTime;
                tempTripTable[k].first.second = stoi(tripDataTable[i][0]);
                tempTripTable[k].second.push_back({destinationKey, rideTimeToDestination, layoverAtDestination, totalTripTime});                
            }
        }
        
        for(int j = 0; j < tripDataTable.size(); j++)
        {
            // skip current index
            if(j == i) continue;
            if(tripDataTable[i][1] == tripDataTable[j][0])
            {
                if(tripDataTable[i][3] < tripDataTable[j][2])
                {                    
                    departureTime = stoi(tripDataTable[j][2]);
                    rideTimeToDestination = stoi(tripDataTable[i][3]) - stoi(tripDataTable[i][2]);
                    layoverAtDestination = stoi(tripDataTable[j][2]) - stoi(tripDataTable[i][3]);
                    totalTripTime = rideTimeToDestination + layoverAtDestination;

                    // Map trip ID to its corresponding key value for easy look up.
                    for(int keyIndx = 0; keyIndx < tripDataTable.size(); keyIndx++)
                    {
                        if(stoi(tripDataTable[keyIndx][0]) == stoi(tripDataTable[j][0]) && stoi(tripDataTable[keyIndx][2]) == stoi(tripDataTable[j][2]))
                        {
                            destinationKey = keyIndx;
                        }
                    }
                    
                    // Search adjacency list for matching vertex to insert new edge data. Can't be a simple index with current setup.
                    // This is the only time this search must happen because when the graph is created, each departure is assigned a lookupKey that can be used
                    // to index the list and match the vertex.
                    for(int k = 0; k < tripDataTable.size(); k++)
                    {
                        //If the target edge departure time and departure station match, this is the correct insertion point, add edge to adjacency list.
                        if(stoi(tripDataTable[k][0]) == stoi(tripDataTable[i][0]) && stoi(tripDataTable[k][2]) == stoi(tripDataTable[i][2]))
                        {                        
                            tempTripTable[k].first.first = departureTime;
                            tempTripTable[k].first.second = stoi(tripDataTable[i][0]);
                            tempTripTable[k].second.push_back({destinationKey, rideTimeToDestination, layoverAtDestination, totalTripTime});
                        }
                    }
                }
            }
        }
    }

    // Populate the departure graph using data from tempTripTable.
    for(int i = 0; i < tempTripTable.size(); i++)
    {
        departureGraphList->push_back({tempTripTable[i].second, tempTripTable[i].first.second, i, tempTripTable[i].first.first});
    }

    // Populate terminating arrival nodes, required for shortest path algortithm
    for(int i = tripDataTable.size(); i < (tripDataTable.size() * 2); i++)
    {
       departureGraphList->push_back({{}, stoi(tripDataTable[i - tripDataTable.size()][0]), i, 0});     
    } 
}

void StationGraph::build_station_arrivals_graph(std::vector<std::vector<std::string>> tripDataTable)
{
    stationArrivalsGraphList = new std::vector<Station>;

    std::vector<std::vector<Trip>> tempTripTable;
    for(int i = 0; i < stationCount; i++)
    {
        tempTripTable.push_back({});
    }

    //Add the trip data to tempTripTable array
    for(int i = 0; i < tripDataTable.size(); i++)
    {
        int startID = stoi(tripDataTable[i][1]) - 1;
        int destinationID = stoi(tripDataTable[i][0]);
        int arrivalTime = stoi(tripDataTable[i][2]);
        int departureTime = stoi(tripDataTable[i][3]);
        int travelTimeMins = (arrivalTime - departureTime);
        tempTripTable[startID].push_back({destinationID, departureTime, arrivalTime, travelTimeMins});
    }

    //Construct the stations and add trips to graph.
    for(int i = 0; i < stationCount; i++)
    {
        stationArrivalsGraphList->push_back({i + 1, tempTripTable[i]});
    }
}

Route StationGraph::get_route(int departureKey, int destinationKey, std::vector<std::vector<int>>& routeLookUpTable)
{        
    std::vector<TripPlusLayover> shortPath;
    
    int nextStopID = departureKey;
    bool endOfPath = false;

    while(!endOfPath)
    {
        Departure currentNode = (*departureGraphList)[nextStopID];
        nextStopID = routeLookUpTable[nextStopID][destinationKey];

        if (currentNode.IsFinalDestination() || nextStopID == Utility::INF)
        {
            endOfPath = true;
        }
        else
        {
            TripPlusLayover nextTrip = currentNode.FindTripByDestinationKey(nextStopID);
            shortPath.push_back(nextTrip);
        }
    }

    Route finalRoute{(*departureGraphList)[departureKey], shortPath, (*departureGraphList)[destinationKey]};

    if(finalRoute.RouteIsValid())
    {
        return finalRoute;
    }
    else
    {
        return{{{}, -1, -1, -1} ,{}, {{}, -1, -1, -1}};
    }            
}

Route StationGraph::get_shortest_route(int departureID, int destinationID, std::vector<std::vector<int>>& routeLookUpTable)
{
    std::vector<Route> potentialRouteList;

    for (int j = 0; j < departureGraphList->size(); j++)
    {
        for (int k = 0; k < departureGraphList->size(); k++)
        {
            if((*departureGraphList)[j].GetStationID() == departureID && (*departureGraphList)[k].GetStationID() == destinationID)
            {
                potentialRouteList.push_back(get_route(j, k, routeLookUpTable));
            }
        }
    }

    //Determine which route is shortest and return it.
    int minimumWeight = Utility::INF;
    int shortestRouteIndex = -1;    
    for(int i = 0; i < potentialRouteList.size(); i++)
    {
        int totalCurrentWeight = 0;
        for(int j = 0; j < potentialRouteList[i].tripList.size(); j++)
        {
            TripPlusLayover currentTrip = potentialRouteList[i].tripList[j];
            totalCurrentWeight += currentTrip.tripWeight;
        }

        if(totalCurrentWeight < minimumWeight)
        {
            minimumWeight = totalCurrentWeight;
            totalCurrentWeight = 0;
            shortestRouteIndex = i;
        }
    }

    return potentialRouteList[shortestRouteIndex];
}

void StationGraph::floyd_warshal_shortest_paths(bool includeLayovers)
{
    const int INF = Utility::INF;
    // Construct adjacency matrix from adjacencyList. If value == INF, no path exists between start and end index.
    std::vector<std::vector<int>> distance(departureGraphList->size(), std::vector<int>(departureGraphList->size(), INF));
    // Sequence table to store shortest paths for future operations.
    std::vector<std::vector<int>>* shortestRouteTable;

    if(includeLayovers)
    {
        shortestRouteWithLayoverSequenceTable = new std::vector<std::vector<int>>(departureGraphList->size(), std::vector<int>(departureGraphList->size(), INF));
        shortestRouteTable = shortestRouteWithLayoverSequenceTable;
    }
    else
    {
        shortestRouteWithoutLayoverSequenceTable = new std::vector<std::vector<int>>(departureGraphList->size(), std::vector<int>(departureGraphList->size(), INF));
        shortestRouteTable = shortestRouteWithoutLayoverSequenceTable;
    }    
    

    for(int i = 0; i < departureGraphList->size(); i++)
    {
        Departure currentDeparture = (*departureGraphList)[i];        
        if(currentDeparture.GetTripCount() == 0)
        {
            // this is a terminating vertex.
            continue;
        }
        else
        {
            for(int j = 0; j < currentDeparture.GetTripCount(); j++)
            {
                int tripWeight = currentDeparture.GetTrip(j).tripWeight;
                int startID = currentDeparture.GetLookUpKey();                
                int destinationID = currentDeparture.GetTrip(j).destinationKey;            

                distance[startID][destinationID] = tripWeight;
                (*shortestRouteTable).at(startID).at(destinationID) = destinationID;
            }   
        }            
    }

    //Floyd Warshal Algorithm
    for(int k = 0; k < departureGraphList->size(); k++)
    {
        for(int i = 0; i < departureGraphList->size(); i++)
        {
            for(int j = 0; j < departureGraphList->size(); j++)
            {
                if(distance[i][k] != INF && distance[k][j] != INF &&
                    distance[i][k] + distance[k][j] < distance[i][j])
                    {
                        distance[i][j] = distance[i][k] + distance[k][j];
                        // Update shortest path table to reflect new shorter node.
                        (*shortestRouteTable).at(i).at(j) = (*shortestRouteTable).at(i).at(k);
                    }
            }
        }
    }
}

Route StationGraph::GetShortestRoute(int departureStationID, int destinationStationID, bool includeLayovers)
{
    if(includeLayovers)
    {
        return get_shortest_route(departureStationID, destinationStationID, *shortestRouteWithLayoverSequenceTable);
    }
    else
    {
        return get_shortest_route(departureStationID, destinationStationID, *shortestRouteWithoutLayoverSequenceTable);
    }
    
}

int StationGraph::GetVertexCount()
{
    return stationCount;
}

Station StationGraph::GetStationFromGraph(int stationID)
{
    int iDAsZeroIndex = stationID - 1;
    if(iDAsZeroIndex < stationsGraphList->size() && iDAsZeroIndex >= 0)
    {
        return (*stationsGraphList)[iDAsZeroIndex];
    }
    else
    {
        // return invalid station if bad station ID
        return Station({-1, {}});
    } 
}

// Duplication of code between two graph types. Might want to pull this out to be more
// generic.
Station StationGraph::GetStationFromArrivalGraph(int stationID)
{
    int iDAsZeroIndex = stationID - 1;
    if(iDAsZeroIndex < stationArrivalsGraphList->size() && iDAsZeroIndex >= 0)
    {
        return (*stationArrivalsGraphList)[iDAsZeroIndex];
    }
    else
    {
        // return invalid station if bad station ID
        return Station({-1, {}});
    } 
}

bool StationGraph::PathExists(int startStationID, int targetStationID)
{
    Station startStation = GetStationFromGraph(startStationID);
    Station targetStation = GetStationFromGraph(targetStationID);

    if(!startStation.StationIsValid() || !targetStation.StationIsValid())
    {
        std::cout << "Invalid station, please try again.\n";
        return false;
    }
    std::vector<bool> discoveredSet(stationCount, false);
    std::queue<Station> frontiereQueue;

    discoveredSet[startStation.GetID() - 1] = true;
    frontiereQueue.push(startStation);

    while(!frontiereQueue.empty())
    {
        Station currentStation = frontiereQueue.front();
        frontiereQueue.pop();

        for(int i = 0; i < currentStation.GetTripCount(); i++)
        {
            // Store current adjacent node for processing.
            Station adjacentStation = GetStationFromGraph(currentStation.GetTrip(i).destinationID);

            // if there is a connection between targetStation and any adjacent node, a path exists from start to target
            if(adjacentStation.GetID() == targetStation.GetID())
            {
                return true;
            }

            // If current adjacent node hasn't already been discovered, add it to frontiere queue and discover it.
            if(!discoveredSet[adjacentStation.GetID() - 1])
            {
                frontiereQueue.push(adjacentStation);
                discoveredSet[adjacentStation.GetID() - 1] = true;
            }
        }
    }
    // Frontiere queue empties before target is reached, no path exists.
    return false;
}

bool StationGraph::DirectPathExists(int station1ID, int station2ID)
{
    Station currentStation = GetStationFromGraph(station1ID);
    for(int i = 0; i < currentStation.GetTripCount(); i++)
    {
        if(currentStation.GetTrip(i).destinationID == station2ID)
        {
            return true;
        }
    }
    return false;
}



