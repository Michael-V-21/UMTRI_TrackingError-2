// UMTRI_TrackingError.cpp : This file contains the 'main' function. Program execution begins and ends there.

//NOTE FOR MR : This is the section where the GPS measurements and the INS roll angle are used to compute tracking error.
//NOTE FOR MR : GPSEndpoints.LatS and GPSEndpoints.LongS are the coordinates of the section starting point.
//NOTE FOR MR : GPSEndpoints.LatE and GPSEndpoints.LongE are the coordinates of the section ending point.
//NOTE FOR MR : GPSEndpoints.<> have to be provided by the user. (We measure those before we start our runs.)
//Precomputed trig functions and constants.
//NOTE FOR MR : REarth(Earth radius) is computed using Latitude.Use 6400000 m for now, and I'll provide a copy of the function.

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <vector>
#include <cstddef>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

struct GPSEndpoints
{
    double LatS;
    double LatE;
    double LongE;
    double LongS;
};

// Get Latitude and Longitude into Sync.
//Sync.Latitude = interp1(double(GpsPos.Time), double(GpsPos.Latitude), double(Sync.Time), 'linear', -999);
//Sync.Longitude = interp1(double(GpsPos.Time), double(GpsPos.Longitude), double(Sync.Time), 'linear', -999);
//Sync = Sync(Sync.Latitude ~= -999, :);

double interp1(const vector<double>& gpsTime, const vector<double>& gpsLocation, double queryTime, double outsideValue)
{
    for (int i = 0; i < gpsTime.size() - 1; i++)
    {
        if (queryTime >= gpsTime[i] && queryTime <= gpsTime[i + 1])
        {
            double t = (queryTime - gpsTime[i]) / (gpsTime[i + 1] - gpsTime[i]);
            return gpsLocation[i] + t * (gpsLocation[i + 1] - gpsLocation[i]);
        }
    }
    return outsideValue;
}

struct Layout
{
    double Lz;
    double Ly;
};

struct Sync
{
    vector<double> Time;
    vector<double>Latitude;
    vector<double>Longitude;

    vector<double> dNorth;
    vector<double> dEast;
    vector<double> dForward;
    vector<double> dLeftward;

    vector<double> INSRoll;
    vector<double> dL;
};

struct INS
{
    vector<double> Time;
    vector<double> INSRoll;
};



int main()
{

    // Earth radius: should use latitiude but use defined measurement for now. 
    double REarth = 6387200.0;

    //GPSEndpoint user input (random numbers in this case for example and testing)
    GPSEndpoints inputGPS = { 38.6159082082639, -89.6422637048284, 38.6158411442181, -89.6386742825292 };

    //REarth = GetEarthRadius(GPSEndpoints.LatS);
    //d2r = pi / 180;
    //cl = cos(GPSEndpoints.LatS * d2r);
    //dN = (GPSEndpoints.LatE - GPSEndpoints.LatS) * d2r * REarth;
    //dE = (GPSEndpoints.LongE - GPSEndpoints.LongS) * d2r * REarth * cl;
    //SegmentLength = sqrt(dE * dE + dN * dN);
    //c = dE / SegmentLength; s = dN / SegmentLength;

    double pi = M_PI;
    const double dr2 = pi / 180.0;
    

    double cl = cos(inputGPS.LatS * dr2);
    double dN = (inputGPS.LatE - inputGPS.LatS) * dr2 * REarth;
    double dE = (inputGPS.LongE - inputGPS.LongS) * dr2 * REarth * cl;
    double segmentLength = sqrt(dE * dE + dN * dN);
    double c = dE / segmentLength;
    double s = dN / segmentLength;

    // Calculating Y (Leftward offset) using CSV file data, and exporting the data back to another file.
    ifstream file("UMTRI-Run2883.csv");
    if (!file.is_open())
    {
        cout << "Error with opening the file" << endl;
    }

    string line;
    //int rowNumber = 0;

    while (getline(file, line))
    {
        if (line.empty()) continue;

        double lat;
        double lon;
        char comma;

        stringstream ss(line);
        if (!(ss >> lat >> comma >> lon)) {
            //cout << "Invalid data at row " << rowNumber << endl;
            continue;
        }

        double dNorth = (lat - inputGPS.LatS) * dr2 * REarth;
        double dEast = (lon - inputGPS.LongS) * dr2 * REarth * cl;
        double dLeftward = (-s * dEast) + (c * dNorth);

        cout << " Y (Leftward offset): " << dLeftward << " meters \n";
        //rowNumber++;

    }

    file.close();




    // Calculating Y (Leftward offset) using user input Latitude and Longitude Pairs
    /*
    string line;
    while (true)
    {
        cout << "Enter Lat and Long Set (Press Enter to quit)" << endl;
        getline(cin, line);

        if (line.empty()) break;

        double lat;
        double lon;

        istringstream iss(line);
        if (!(iss >> lat >> lon))
        {
            cout << "Invalid input" << endl;
            continue;
        }

        double dNorth = (lat - inputGPS.LatS) * dr2 * REarth;
        double dEast = (lon - inputGPS.LongS) * dr2 * REarth * cl;
        double dLeftward = (- s * dEast) + (c * dNorth);

        cout << "Y (Leftward offset): " << dLeftward << " meters \n";

    }
    */

    //Previous Example GPS data
    /*
    //Example time, location and querytime data
    vector<double> gpsTime = { 0.0, 1.0, 2.0, 3.0 };
    vector<double> gpsLat = { 10.0, 20.0, 30.0, 40.0 };
    vector<double> gpsLon = { 100.0, 110.0, 120.0, 130.0 };
    vector<double> syncTime = { 0.5, 1.5, 2.5, 5.0 };

    // Get Latitude and Longitude into Sync.
    //Sync.Latitude = interp1(double(GpsPos.Time), double(GpsPos.Latitude), double(Sync.Time), 'linear', -999);
    //Sync.Longitude = interp1(double(GpsPos.Time), double(GpsPos.Longitude), double(Sync.Time), 'linear', -999);
    //Sync = Sync(Sync.Latitude ~= -999, :);

    Sync sync;
    sync.Time = syncTime;

    for (double t : syncTime)
    {
        double lat = interp1(gpsTime, gpsLat, t, -999);
        double lon = interp1(gpsTime, gpsLon, t, -999);

        if (lat != -999 && lon != -999)
        {
            sync.Latitude.push_back(lat);
            sync.Longitude.push_back(lon);
        }
    }

    //% Make signals that define the profiler path relative to the desired line.
    //Sync.dNorth = (Sync.Latitude - GPSEndpoints.LatS) * d2r * REarth;
    //Sync.dEast = (Sync.Longitude - GPSEndpoints.LongS) * d2r * REarth * cl;
    //% Forward and leftward.
    //
    //% NOTE FOR MR : The two items below are the position along the section and the position leftward of the desired track
    //% of the GPS antenna.The antenna is not in the same location as the profiler height sensor footprint on the ground,
    //% so, more calculations follow to make the adjustment.
    //Sync.dForward = c * Sync.dEast + s * Sync.dNorth;
    //Sync.dLeftward = -s * Sync.dEast + c * Sync.dNorth;

    for (int i = 0; i < sync.Latitude.size(); i++)
    {
        double lat = sync.Latitude[i];
        double lon = sync.Longitude[i];

        double dNorth = (lat - inputGPS.LatS) * dr2 * REarth;
        double dEast = (lon - inputGPS.LongS) * dr2 * REarth * cl;

        sync.dNorth.push_back(dNorth);
        sync.dEast.push_back(dEast);
    }

    for (int i = 0; i < sync.dEast.size(); i++)
    {
        double dForward = c * sync.dEast[i] + s * sync.dNorth[i];
        double dLeftward = -s * sync.dEast[i] + c * sync.dNorth[i];

        sync.dForward.push_back(dForward);
        sync.dLeftward.push_back(dLeftward);
    }

    //% Interpolate INS Roll to the clock of Sync.
    //Sync.INSRoll = interp1(double(Ins.Time), Ins.INSRoll, double(Sync.Time));
    //
    //% Get profiler goemtry.
    //% NOTE FOR MR : Lz and Ly are user entered. (They will not change from run to run, though.)
    //% They are the vertical and lateral position of the profiler height sensor footprint relative to the GPS antenna.
    //Lz = Layout.Lz;% Z coord of the ground in the frame of the Gps antenna.
    //Ly = Layout.Ly;% Y coord of the ground in the frame of the Gps antenna.
    //
    //% Project the GPS location to the ground using the roll angle.
    //% NOTE FOR MR : This is the adjustment mentioned above.It requires the roll angle measured by the INS.
    //Sync.dL = Sync.dLeftward - Lz * sin(Sync.INSRoll* d2r) + Ly * cos(Sync.INSRoll* d2r);
    //% NOTE FOR MR : This is the end of the tracking stuff in this function.




    INS ins;
    ins.Time = { 0.0, 1.0, 2.0, 3.0 };
    ins.INSRoll = { 0.0, 5.0, -3.0, 2.0 };


    for (double t : sync.Time)
    {
        double roll = interp1(ins.Time, ins.INSRoll, t, -999);
        sync.INSRoll.push_back(roll);
    }

    double Lz = 1.0;
    double Ly = 0.5;

    for (int i = 0; i < sync.dLeftward.size(); i++)
    {
        double roll = sync.INSRoll[i];

        double rollRad = roll * dr2;

        double dL = sync.dLeftward[i]
            - Lz * sin(rollRad)
            + Ly * cos(rollRad);

        sync.dL.push_back(dL);
    }
    */

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
