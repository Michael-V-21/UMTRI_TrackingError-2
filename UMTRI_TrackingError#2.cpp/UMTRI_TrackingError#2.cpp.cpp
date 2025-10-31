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
#include <iomanip>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::stringstream;
using std::fixed;

struct GPSEndpoints
{
    double LatS;
    double LongS;
    double LatE;
    double LongE;
};

// Get Latitude and Longitude into Sync.
//Sync.Latitude = interp1(double(GpsPos.Time), double(GpsPos.Latitude), double(Sync.Time), 'linear', -999);
//Sync.Longitude = interp1(double(GpsPos.Time), double(GpsPos.Longitude), double(Sync.Time), 'linear', -999);
//Sync = Sync(Sync.Latitude ~= -999, :);

double interp1(const vector<double>& gpsTime, const vector<double>& gpsLocation, double queryTime, double outsideValue)
{
    //New stuff 10/31
    // if gpsTime is empty or only one point, handle gracefully
    if (gpsTime.empty() || gpsLocation.empty()) return outsideValue;
    if (gpsTime.size() == 1) return gpsLocation[0];

    for (int i = 0; i < gpsTime.size() - 1; i++)
    {
        if (queryTime >= gpsTime[i] && queryTime <= gpsTime[i + 1])
        {
            double t = (queryTime - gpsTime[i]) / (gpsTime[i + 1] - gpsTime[i]);
            return gpsLocation[i] + t * (gpsLocation[i + 1] - gpsLocation[i]);
        }
    }
    //return outsideValue;
    if (queryTime < gpsTime.front()) return gpsLocation.front();
    return gpsLocation.back();
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


/*
int main()
{
    cout << fixed << setprecision(13);

    // Earth radius: should use latitiude but use defined measurement for now. 
    double REarth = 6387200.0;

    //GPSEndpoint user input (random numbers in this case for example and testing)
    GPSEndpoints inputGPS = { 38.6159095515001 , -89.6422556192682, 38.6158436940279, -89.6386750652108 };

    double pi = M_PI;
    const double dr2 = pi / 180.0;


    //double cl = cos(inputGPS.LatS * dr2);
    double cl = cos(((inputGPS.LatS + inputGPS.LatE) / 2.0) * dr2);

    double dN = (inputGPS.LatE - inputGPS.LatS) * dr2 * REarth;
    double dE = (inputGPS.LongE - inputGPS.LongS) * dr2 * REarth * cl;
    double segmentLength = sqrt(dE * dE + dN * dN);
    double c = dE / segmentLength;
    double s = dN / segmentLength;

    double Lz = 1.0;
    double Ly = 0.5;

    // Calculating Y (Leftward offset) using CSV file data, and exporting the data back to another file.
    ifstream file("UMTRI-RUN2883-time,insrol,lat,long,time.csv");
    ofstream out("UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv");

    if (!file.is_open() || !out.is_open())
    {
        cout << "Error with opening the file" << endl;
    }

    // CSV headers
    out << "Time,INSRoll,Latitude,Longitude,dLeftward, ,dForward, dL\n";

    string line;
    bool firstLine = true;

    bool hasPrev = false;
    double prevLat = 0.0;
    double prevLong = 0.0;

    vector<double> rawTime;
    vector<double> rawLat;
    vector<double> rawLon;
    vector<double> rawInsRoll;

    
    while (getline(file, line))
    {
        if (line.empty()) continue;

        double iTime, iInsRoll, gLat, gLon;
        char comma;
        string cell;

        stringstream ss(line);

        if (!(ss >> iTime >> comma >> gLat >> comma >> gLon >> comma >> iInsRoll )) {
            cout << "Skipping invalid row: " << line << endl;
            continue;
        }

        if (firstLine) {
            cout << "First CSV Lat/Lon: " << gLat << ", " << gLon << endl;
            firstLine = false;
        }

        double dNorth = (gLat - inputGPS.LatS) * dr2 * REarth;

        //double cl_current = cos(gLat * dr2);
        double dEast = (gLon - inputGPS.LongS) * dr2 * REarth * cl;

        double dLeftward = (-s * dEast) + (c * dNorth);
        double dForward = c * dEast + s * dNorth;

        static const double dForwardStart = -96.02691236;
        dForward += dForwardStart;

        //static const double dLeftwardStart = 0.1461333714;
        //dLeftward += dLeftwardStart;

        // Profiler geometry
        double Lz = 1.0;
        double Ly = 0.5;
        double rollRad = iInsRoll * dr2;
        double dL = dLeftward - Lz * sin(rollRad) + Ly * cos(rollRad);

        static const double dLStart = -0.2732048853;
        dL += dLStart;

        //cout << " Lat: " << lat << " Lon: " << lon << "   |   Y (Leftward offset) : " << dLeftward << " meters \n";
        //out << lat << ", " << lon << ", " << dLeftward << endl;
        out << fixed << setprecision(13)
            << iTime << ", "
            << iInsRoll << ", "
            << gLat << ", "
            << gLon << ", "
            << dLeftward << ", "
            << "      " << ", "
            << dForward << ", "
            << dL
            << endl;

    }


    file.close();
    out.close();
    cout << "Y offset results saved to UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv \n";

}
*/

//Best one so far, shifted up
/*
int main()
{
    cout << fixed << setprecision(13);

    // Earth radius (in meters)
    double REarth = 6387200.0;

    // GPS endpoints (rear left antenna start/end)
    GPSEndpoints inputGPS = { 38.6159095515001 , -89.6422556192682,
                              38.6158436940279, -89.6386750652108 };

    double pi = M_PI;
    const double dr2 = pi / 180.0;

    // Profiler geometry
    double Lz = 1.0;
    double Ly = 0.5;

    // File I/O
    ifstream file("UMTRI-RUN2883-time,insrol,lat,long,time.csv");
    ofstream out("UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv");

    if (!file.is_open() || !out.is_open())
    {
        cout << "Error with opening the file" << endl;
        return 1;
    }

    // CSV headers
    out << "Time,INSRoll,Latitude,Longitude,dLeftward, ,dForward, dL\n";

    vector<double> rawTime, rawLat, rawLon, rawInsRoll;
    string line;
    //bool firstLine = true;

    // --- STEP 1: Read all points ---
    while (getline(file, line))
    {
        if (line.empty()) continue;

        double iTime, iInsRoll, gLat, gLon;
        char comma;
        stringstream ss(line);

        if (!(ss >> iTime >> comma >> gLat >> comma >> gLon >> comma >> iInsRoll))
        {
            cout << "Skipping invalid row: " << line << endl;
            continue;
        }

        rawTime.push_back(iTime);
        rawLat.push_back(gLat);
        rawLon.push_back(gLon);
        rawInsRoll.push_back(iInsRoll);
    }
    file.close();

    // --- STEP 2: Compute best-fit line (Lon vs Lat) ---
    // Linear regression: Lon = m*Lat + b
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    int N = rawLat.size();
    for (int i = 0; i < N; i++)
    {
        sumX += rawLat[i];
        sumY += rawLon[i];
        sumXY += rawLat[i] * rawLon[i];
        sumXX += rawLat[i] * rawLat[i];
    }
    double m = (N * sumXY - sumX * sumY) / (N * sumXX - sumX * sumX);
    double b = (sumY - m * sumX) / N;

    // Compute start point projection
    double lat0 = rawLat.front();
    double lon0 = m * lat0 + b;

    double cumulativeForward = 0.0;

    // --- STEP 3: Compute dForward and dLeftward relative to best-fit line ---
    for (int i = 0; i < N; i++)
    {
        double gLat = rawLat[i];
        double gLon = rawLon[i];
        double iTime = rawTime[i];
        double iInsRoll = rawInsRoll[i];

        // Project point onto best-fit line
        // Line vector in meters
        double dLat_line = gLat - lat0;
        double dLon_line = gLon - lon0;

        double dNorth_line = dLat_line * dr2 * REarth;
        double dEast_line = dLon_line * dr2 * REarth * cos((gLat + lat0) / 2.0 * dr2);

        // Direction vector of line
        double lat1 = rawLat.back();
        double lon1 = rawLon.back();
        double dN_line_total = (lat1 - lat0) * dr2 * REarth;
        double dE_line_total = (lon1 - lon0) * dr2 * REarth * cos((lat1 + lat0) / 2.0 * dr2);
        double lineLength = sqrt(dN_line_total * dN_line_total + dE_line_total * dE_line_total);

        double c = dE_line_total / lineLength;
        double s = dN_line_total / lineLength;

        // Forward and leftward (perpendicular)
        double dForward = c * dEast_line + s * dNorth_line;
        double dLeftward = -s * dEast_line + c * dNorth_line;

        // Apply start offset to match previous runs
        static const double dForwardStart = -96.02691236;
        dForward += dForwardStart;

        // Compute dL using profiler geometry and roll
        double rollRad = iInsRoll * dr2;
        double dL = dLeftward - Lz * sin(rollRad) + Ly * cos(rollRad);

        static const double dLStart = -0.2732048853;
        dL += dLStart;

        // Write output
        out << fixed << setprecision(13)
            << iTime << ", "
            << iInsRoll << ", "
            << gLat << ", "
            << gLon << ", "
            << dLeftward << ", "
            << "      " << ", "
            << dForward << ", "
            << dL
            << endl;
    }

    out.close();
    cout << "Y offset results saved to UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv\n";
}
*/

int main()
{
    cout << fixed << setprecision(13);

    // Earth radius (in meters)
    double REarth = 6387200.0;

    // GPS endpoints (rear left antenna start/end)
    GPSEndpoints inputGPS = { 38.6159095515001 , -89.6422556192682,
                              38.6158436940279, -89.6386750652108 };

    double pi = M_PI;
    const double dr2 = pi / 180.0;

    // Profiler geometry
    double Lz = 1.0;
    double Ly = 0.5;

    // File I/O
    ifstream file("UMTRI-RUN2883-time,insrol,lat,long,time.csv");
    ofstream out("UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv");

    if (!file.is_open() || !out.is_open())
    {
        cout << "Error with opening the file" << endl;
        return 1;
    }

    // CSV headers
    out << "Time,INSRoll,Latitude,Longitude,dLeftward, ,dForward, dL\n";

    vector<double> rawTime, rawLat, rawLon, rawInsRoll;
    string line;

    // --- STEP 1: Read all points ---
    while (getline(file, line))
    {
        if (line.empty()) continue;

        double iTime, iInsRoll, gLat, gLon;
        char comma;
        stringstream ss(line);

        if (!(ss >> iTime >> comma >> gLat >> comma >> gLon >> comma >> iInsRoll))
        {
            cout << "Skipping invalid row: " << line << endl;
            continue;
        }

        rawTime.push_back(iTime);
        rawLat.push_back(gLat);
        rawLon.push_back(gLon);
        rawInsRoll.push_back(iInsRoll);
    }
    file.close();

    // --- STEP 2: Compute best-fit line (Lon vs Lat) ---
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    int N = rawLat.size();
    for (int i = 0; i < N; i++)
    {
        sumX += rawLat[i];
        sumY += rawLon[i];
        sumXY += rawLat[i] * rawLon[i];
        sumXX += rawLat[i] * rawLat[i];
    }
    double m = (N * sumXY - sumX * sumY) / (N * sumXX - sumX * sumX);
    double b = (sumY - m * sumX) / N;

    double lat0 = rawLat.front();
    double lon0 = m * lat0 + b;

    double cumulativeForward = 0.0;
    double dLStart = 0.0;
    bool firstPoint = true;

    // --- STEP 3: Compute dForward and dLeftward relative to best-fit line ---
    for (int i = 0; i < N; i++)
    {
        double gLat = rawLat[i];
        double gLon = rawLon[i];
        double iTime = rawTime[i];
        double iInsRoll = rawInsRoll[i];

        // Project point onto best-fit line
        double dLat_line = gLat - lat0;
        double dLon_line = gLon - lon0;

        double dNorth_line = dLat_line * dr2 * REarth;
        double dEast_line = dLon_line * dr2 * REarth * cos((gLat + lat0) / 2.0 * dr2);

        // Direction vector of line
        double lat1 = rawLat.back();
        double lon1 = rawLon.back();
        double dN_line_total = (lat1 - lat0) * dr2 * REarth;
        double dE_line_total = (lon1 - lon0) * dr2 * REarth * cos((lat1 + lat0) / 2.0 * dr2);
        double lineLength = sqrt(dN_line_total * dN_line_total + dE_line_total * dE_line_total);

        double c = dE_line_total / lineLength;
        double s = dN_line_total / lineLength;

        // Forward and leftward (perpendicular)
        double dForward = c * dEast_line + s * dNorth_line;
        double dLeftward = -s * dEast_line + c * dNorth_line;

        // Apply start offset to match previous runs
        static const double dForwardStart = -96.02691236;
        dForward += dForwardStart;

        // Compute dL using profiler geometry and roll
        double rollRad = iInsRoll * dr2;
        double dL = dLeftward - Lz * sin(rollRad) + Ly * cos(rollRad);

        // Shift dL so first point starts at 0.21
        if (firstPoint)
        {
            double desiredStart = 0.2101397687;
            dLStart = desiredStart - dL;
            firstPoint = false;
        }
        dL += dLStart;

        // Write output
        out << fixed << setprecision(13)
            << iTime << ", "
            << iInsRoll << ", "
            << gLat << ", "
            << gLon << ", "
            << dLeftward << ", "
            << "      " << ", "
            << dForward << ", "
            << dL
            << endl;
    }

    out.close();
    cout << "Y offset results saved to UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv\n";
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