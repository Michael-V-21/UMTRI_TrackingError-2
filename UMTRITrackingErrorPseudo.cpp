/*

% This function creates a record of leftward tracking bias of the left
% profiler footprint and datron distance versus time.
% The GPS endponits are used to set the desired track, and to set the
% distance channel to zero where the starting point is crossed.
% Latitude and longutide of the top left GPS antennna versus time are
% obtained from GpsMasterPosOut.
% Body roll versus time is obtained from from InsOut.
% Lat, long, and roll are interpolated into the Sync table to get a fast
% clock.
% The output will be used to get leftward bias versus time and a longitudinal
% distance channel with a proper zero into DataKHz.
% In a less diagnostic version of the code, this step could be hidden as long
% as valid Distance and dL signal appear in DataKHz.
% Note that the geometry of the ULSP(the location of the GPS antenna
    % relative to the profiler footprint; Ly and Lz) is hard coded here.Those
    % dimensions should have been treated as input parameters.
    %
    % If this is a static run or the GPS endpoints are not known, return
    % leftward tracking bias of zero.
    % If this is a static run, return an artificial distance signal using
    % StaticRunSpeed.
    % If the GPS endpoints are not known, set distance reference at the tat
    % of the run.
    function[Tracking, StaticRun, RunStats] = GetTrackingvTime(GPSEndpoints, Sync, GpsPos, Ins, Layout, Flags)

    % Set the DatronDistanceCount calibration factor.This should really be a
    % passed parameter.
    DatronMetersperPulse = 0.002;

% Determine whether this is a static run.
StaticRun = false;
if (max(Sync.DatronDistanceCount) - min(Sync.DatronDistanceCount) < 250)
    StaticRun = true;
if (Flags.EchoInfo)
fprintf('INFO: This is a static run.\n');
end
end

% If this is a static run.
if (StaticRun)
Tracking = Sync(:, { 'Time' });
time0 = double(Sync.Time(1));
% This should be in settings, but for now it is here.
% If a static run is less than 26 second long, assume the static run
% speed should be 50 mph.
% Otherwise, use 25 mph.
if (Sync.Time(end) - Sync.Time(1) > 26000)
StaticRunSpeed = 25 * 5280 * 12 * 0.0254 / 3600;
else
StaticRunSpeed = 50 * 5280 * 12 * 0.0254 / 3600;
end
Tracking.Distance = (Tracking.Time - time0) * StaticRunSpeed;
Tracking.dL(:) = 0;
% Store stuff in a structure for cleaner passing.
RunStats = struct('SpeedAve', StaticRunSpeed, 'SpeedMin', StaticRunSpeed, ...
    'dLAve', 0, 'dLAbsAve', 0, 'dLStDev', 0, 'XStart', 0, 'XEnd', 0);
return;
end

% If the GPS endpoints are not known.
if (GPSEndpoints.LatE == 0)
Tracking = Sync(:, { 'Time' });
DDCStart = double(Sync.DatronDistanceCount(1));
Tracking.Distance = DatronMetersperPulse * (double(Sync.DatronDistanceCount) - DDCStart);
Tracking.dL(:) = 0;
% Store stuff in a structure for cleaner passing.
RunStats = struct('SpeedAve', mean(Ins.INSSpeed), 'SpeedMin', min(Ins.INSSpeed), ...
    'dLAve', 0, 'dLAbsAve', 0, 'dLStDev', 0, 'XStart', 0, 'XEnd', 0);
return;
end

*/

% NOTE FOR MR : This is the section where the GPS measurements and the INS roll angle are
% used to compute tracking error.
% NOTE FOR MR : GPSEndpoints.LatS and GPSEndpoints.LongS are the coordinates of the section starting point.
% NOTE FOR MR : GPSEndpoints.LatE and GPSEndpoints.LongE are the coordinates of the section ending point.
% NOTE FOR MR : GPSEndpoints.<> have to be provided by the user. (We measure those before we start our runs.)
% Precomputed trig functions and constants.
% NOTE FOR MR : REarth(Earth radius) is computed using Latitude.Use 6400000 m for now, and I'll
% provide a copy of the function.
REarth = GetEarthRadius(GPSEndpoints.LatS);
d2r = pi / 180;
cl = cos(GPSEndpoints.LatS * d2r);
dN = (GPSEndpoints.LatE - GPSEndpoints.LatS) * d2r * REarth;
dE = (GPSEndpoints.LongE - GPSEndpoints.LongS) * d2r * REarth * cl;
SegmentLength = sqrt(dE * dE + dN * dN);
c = dE / SegmentLength; s = dN / SegmentLength;

% Get Latitude and Longitude into Sync.
Sync.Latitude = interp1(double(GpsPos.Time), double(GpsPos.Latitude), double(Sync.Time), 'linear', -999);
Sync.Longitude = interp1(double(GpsPos.Time), double(GpsPos.Longitude), double(Sync.Time), 'linear', -999);
Sync = Sync(Sync.Latitude ~= -999, :);

% Make signals that define the profiler path relative to the desired line.
Sync.dNorth = (Sync.Latitude - GPSEndpoints.LatS) * d2r * REarth;
Sync.dEast = (Sync.Longitude - GPSEndpoints.LongS) * d2r * REarth * cl;
% Forward and leftward.
% NOTE FOR MR : The two items below are the position along the section and the position leftward of the desired track
% of the GPS antenna.The antenna is not in the same location as the profiler height sensor footprint on the ground,
% so, more calculations follow to make the adjustment.
Sync.dForward = c * Sync.dEast + s * Sync.dNorth;
Sync.dLeftward = -s * Sync.dEast + c * Sync.dNorth;

% Interpolate INS Roll to the clock of Sync.
Sync.INSRoll = interp1(double(Ins.Time), Ins.INSRoll, double(Sync.Time));

% Get profiler goemtry.
% NOTE FOR MR : Lz and Ly are user entered. (They will not change from run to run, though.)
% They are the vertical and lateral position of the profiler height sensor footprint relative to the GPS antenna.
Lz = Layout.Lz;% Z coord of the ground in the frame of the Gps antenna.
Ly = Layout.Ly;% Y coord of the ground in the frame of the Gps antenna.

% Project the GPS location to the ground using the roll angle.
% NOTE FOR MR : This is the adjustment mentioned above.It requires the roll angle measured by the INS.
Sync.dL = Sync.dLeftward - Lz * sin(Sync.INSRoll* d2r) + Ly * cos(Sync.INSRoll* d2r);
% NOTE FOR MR : This is the end of the tracking stuff in this function.

/*

% Return a table of dL versus time.
Tracking = Sync(:, { 'Time' 'dL' });

% Find DatronDistanceCount at the start of the section.
DDCStart = interp1(double(Sync.dForward), double(Sync.DatronDistanceCount), 0.0, 'linear', -999);
Tracking.Distance = DatronMetersperPulse * (double(Sync.DatronDistanceCount) - DDCStart);
% Get the minimum speed and average speed within the test section.
TStart = interp1(double(Sync.dForward), double(Sync.Time), 0.0, 'linear', -999999);
TEnd = interp1(double(Sync.dForward), double(Sync.Time), GPSEndpoints.SegmentLength, 'linear', 999999);
SpeedAve = mean(Ins.INSSpeed(Ins.Time > TStart & Ins.Time < TEnd));
SpeedMin = min(Ins.INSSpeed(Ins.Time > TStart & Ins.Time < TEnd));
dLAve = mean(Tracking.dL(double(Tracking.Time) > TStart & double(Tracking.Time) < TEnd));
dLStDev = std(Tracking.dL(double(Tracking.Time) > TStart & double(Tracking.Time) < TEnd));
dLAbsAve = mean(abs(Tracking.dL(double(Tracking.Time) > TStart & double(Tracking.Time) < TEnd)));
if (Flags.EchoInfo)
fprintf('INFO: Average speed %s.\n', num2str(SpeedAve));
fprintf('INFO: Minimum speed %s.\n', num2str(SpeedMin));
end

% Store stuff in a structure for cleaner passing.
RunStats = struct('SpeedAve', SpeedAve, 'SpeedMin', SpeedMin, ...
    'dLAve', dLAve, 'dLAbsAve', dLAbsAve, 'dLStDev', dLStDev, ...
    'XStart', Tracking.Distance(1), 'XEnd', Tracking.Distance(end));

end

*/
