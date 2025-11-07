// UMTRI_TrackingError.cpp : This file contains the 'main' function. Program execution begins and ends there.

//NOTE FOR MR : This is the section where the GPS measurements and the INS roll angle are used to compute tracking error.
//NOTE FOR MR : GPSEndpoints.LatS and GPSEndpoints.LongS are the coordinates of the section starting point.
//NOTE FOR MR : GPSEndpoints.LatE and GPSEndpoints.LongE are the coordinates of the section ending point.
//NOTE FOR MR : GPSEndpoints.<> have to be provided by the user. (We measure those before we start our runs.)
//Precomputed trig functions and constants.
//NOTE FOR MR : REarth(Earth radius) is computed using Latitude.Use 6400000 m for now, and I'll provide a copy of the function.


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
