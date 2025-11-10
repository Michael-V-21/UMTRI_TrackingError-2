
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


int main()
{
    cout << fixed << setprecision(13);

    // Earth radius: should use latitiude but use defined measurement for now.
    double REarth = 6387200.0;

    //GPSEndpoint user input (random numbers in this case for example and testing)
    //GPSEndpoints inputGPS = { 38.61590955 , -89.64225562, 38.61584369, -89.63867507 }; 
    GPSEndpoints inputGPS = { 38.61588714 , -89.64115053, 38.61585436, -89.63930307};

    double pi = M_PI;
    const double dr2 = pi / 180.0;


    double cl = cos(inputGPS.LatS * dr2);
    //double cl = cos(((inputGPS.LatS + inputGPS.LatE) / 2.0) * dr2);

    double dN = (inputGPS.LatE - inputGPS.LatS) * dr2 * REarth;
    double dE = (inputGPS.LongE - inputGPS.LongS) * dr2 * REarth * cl;
    double segmentLength = sqrt(dE * dE + dN * dN);
    double c = dE / segmentLength;
    double s = dN / segmentLength;

    // Calculating Y (Leftward offset) using CSV file data, and exporting the data back to another file.
    ifstream file("UMTRI-RUN2883-time,insrol,lat,long,time.csv");
    ofstream out("UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv");

    if (!file.is_open() || !out.is_open())
    {
        cout << "Error with opening the file" << endl;
    }

    // CSV headers
    out << "Time,INSRoll,Latitude,Longitude,dLeftward, ,dForward, dL,  ,dNorth, dEast\n";

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

        // Prints time, lat, long, roll to vs output terminal
        /*
        cout << "Row read -> Time: " << iTime
            << "  Lat: " << gLat
            << "  Lon: " << gLon
            << "  Roll: " << iInsRoll << endl;
        */

        if (firstLine) {
            cout << "First CSV Lat/Lon: " << gLat << ", " << gLon << endl;
            firstLine = false;
        }

        double cl_current = cos(gLat * dr2);

        //double cl_current = cos(gLat * dr2);
        double dNorth = (gLat - inputGPS.LatS) * dr2 * REarth;
        double dEast = (gLon - inputGPS.LongS) * dr2 * REarth * cl_current;

        // Print dNorth and dEast to vs output terminal to check
        //cout << "Computed -> dNorth: " << dNorth
        //    << "  dEast: " << dEast << endl;

        double dLeftward = (-s * dEast) + (c * dNorth);
        double dForward = c * dEast + s * dNorth;

        static const double dForwardStart = -96.02691236;
        dForward += dForwardStart;

        static const double dEastStart = -95.99508083;
        dEast += dEastStart;

        static const double dNorthStart = 2.491988945;
        dNorth += dNorthStart;

        static const double dLeftwardStart = 0.3124987224;
        dLeftward += dLeftwardStart;

        // Profiler geometry
        double Lz = -2.2;
        double Ly = -0.1385;

        double dL = dLeftward - Lz * sin(iInsRoll * dr2) + Ly * cos(iInsRoll * dr2);

        //static const double dLStart = -0.5858722893;
        //dL += dLStart;

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
            << dL << ", "
            <<"       " << ", "
            << dNorth << ", "
            << dEast << ", "
            << endl;


    }


    file.close();
    out.close();
    cout << "Y offset results saved to UMTRI-Run2883-time,insrol,lat,long,time-YResults.csv \n";

}

