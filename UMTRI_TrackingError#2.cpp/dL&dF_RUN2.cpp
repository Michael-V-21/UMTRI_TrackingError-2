// UMTRI_TrackingError.cpp : This file contains the 'main' function. Program execution begins and ends there.

//NOTE FOR MR : This is the section where the GPS measurements and the INS roll angle are used to compute tracking error.
//NOTE FOR MR : GPSEndpoints.LatS and GPSEndpoints.LongS are the coordinates of the section starting point.
//NOTE FOR MR : GPSEndpoints.LatE and GPSEndpoints.LongE are the coordinates of the section ending point.
//NOTE FOR MR : GPSEndpoints.<> have to be provided by the user. (We measure those before we start our runs.)
//Precomputed trig functions and constants.
//NOTE FOR MR : REarth(Earth radius) is computed using Latitude.Use 6400000 m for now, and I'll provide a copy of the function.



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
