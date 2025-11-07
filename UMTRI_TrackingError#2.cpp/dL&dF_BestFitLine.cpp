// Best - fit line version

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

    // best-fit line
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

    // dForward and dLeftward relative to best-fit line
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


        double dForward = c * dEast_line + s * dNorth_line;
        double dLeftward = -s * dEast_line + c * dNorth_line;

        // dForward distance offset
        static const double dForwardStart = -96.02691236;
        dForward += dForwardStart;


        double rollRad = iInsRoll * dr2;
        double dL = dLeftward - Lz * sin(rollRad) + Ly * cos(rollRad);

        // Shifted dL so first point starts at steve's first dL
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
*/