/*
enum BRAKEDECISIONSTATES {
	BDS_NEWINTERSECT, BDS_SAME_INTERSECT, BDS_REDLIGHT, BDS_STOPSIGN,
	BDS_ALREADYBRAKING, BDS_YELLOW, BDS_GREEN
};
BYTE f_DecisionToBrake(Frame* fr)
{
	static BRAKEDECISIONSTATES bdsState = BDS_NEWINTERSECT;
	char szBrakeMsg[BRAKE_MSG_SIZE];
	BYTE bVal = 0;
	BYTE bStopVal = 0;
	BYTE bNewPerceptionData;
	float fDist;
	int nMaxDist;
	int nLightState = 0;
	int nDecDist = 0;
	float fConf = 0;
	float fSpeed = 0;
	CanCard* KvaserCard;
	unsigned long lIdMsg = 0x10d;
	unsigned long lPosLocalIdMsg = 0x10c;
	static float fPrevSpeed = 0;
	static int nPrevLightState = 0;
	static float fPrevConf = 0;
	static BYTE bPrevBrakeDecision = 0;
	static BYTE bPrevStopDecision = 0;
	static float fPrevDist = 1000;
	float fCalcStartToStopLoc = 30;
	static BYTE bPlayBrakeWarn = false;
	BYTE nSecs = 0;
	BYTE bDynDistBraking = 0;
	BYTE nBrakingDist = 30;
	int nPlayNum = 0;
	BYTE bBrakingAlready = false;
	BYTE bNewGps = false;
	unsigned long long nIntNodeId;
	static unsigned long long nPrevIntNodeId = 0;
	static unsigned long long nBrakeWarnNodeId = 0;
	static unsigned long long nBrakeBrakeNodeId = 0;
	static float fTimeYellow = 0;  // we're running 200 hz

	// we only care about making any new braking decisions after we've received some new perception data
	// otherwise we will continue to use our previous perception data
	bNewPerceptionData = FromLinuxNew->GetValue(fr);
	if (bNewPerceptionData)
	{
		nLightState = DetectBoxLightState->GetValue(fr);
		fConf = DetectBoxLightStateConf->GetValue(fr);
	}

	bNewGps = GpsNew->GetValue(fr);
	if (bNewGps)
	{
		fSpeed = Speed->GetValue(fr);
		fDist = DistToIntersection->GetValue(fr);
		// since the vehicle's GPS antenna is in the back of the car, subtract 4 meters
		fDist = fDist - CAR_LENGTH_TO_ANTENNA;
		nIntNodeId = IntersectionNodeId->GetValue(fr);
		if (nPrevIntNodeId != nIntNodeId || (fPrevDist + 5) < fDist || nIntNodeId == 0)  // if it looks like we are looking at the next int
			bdsState = BDS_NEWINTERSECT;



	}
	else // use all the previous data for GPS stuff
	{
		nIntNodeId = nPrevIntNodeId;
		fDist = fPrevDist;
		fSpeed = fPrevSpeed;
		// do not change or set the bdsState
	}

	// if we don't have any new data, GPS or perception, then we don't have
	// to do any new decision making
	if (!bNewGps && !bNewPerceptionData)
	{
		return m_bBrakeDec;
	}

	// get some static values we'll need later
	nSecs = SecBeforeBrakeWarn->GetValue(fr);
	bDynDistBraking = Scenario->GetValue(fr);
	nBrakingDist = BaseBrakingDist->GetValue(fr);
	nDecDist = DecisionDist->GetValue(fr);

	if (bDynDistBraking)
	{
		if (fSpeed > 11)
			fCalcStartToStopLoc = nBrakingDist + (fSpeed - 11) * 5;
		else
			fCalcStartToStopLoc = nBrakingDist;
	}
	else
		fCalcStartToStopLoc = nBrakingDist;




	KvaserCard = can.GetCanCard();

	bBrakingAlready = BrakingAlready->GetValue(fr);


	switch (bdsState)
	{
	case BDS_NEWINTERSECT:
		if (bNewPerceptionData)
		{
			nPrevLightState = nLightState;
			fPrevConf = fConf;
		} // if New light state data

		bVal = 0;  // can't brake with just one sample
		nBrakeWarnNodeId = 0;
		nBrakeBrakeNodeId = 0;
		bPrevBrakeDecision = 0;
		bPrevStopDecision = 0;
		fTimeYellow = 0;
		bPlayBrakeWarn = false;

		if (nPrevIntNodeId == nIntNodeId && nIntNodeId != 0)
			bdsState = BDS_SAME_INTERSECT;
		break;

	case BDS_SAME_INTERSECT:
		fTimeYellow = 0;
		if (bNewPerceptionData)
		{
			if (nLightState == nPrevLightState && (fPrevConf >= MIN_LIGHT_CONF || fConf >= MIN_LIGHT_CONF))
			{
				if (nLightState == LTST_RED)
					bdsState = BDS_REDLIGHT;
				else if (nLightState == LTST_YELLOW)
					bdsState = BDS_YELLOW;
				else if (nLightState == LTST_GREEN)
					bdsState = BDS_GREEN;
				else if (nLightState == LTST_STOPSIGN)
					bdsState = BDS_STOPSIGN;

			}
			nPrevLightState = nLightState;
			fPrevConf = fConf;

		} // if New light state data
		// if we know we are approaching a stop sign, we can just go to that code
		if (sClosestIntersection.stType == ST_STOPSIGN)
			bdsState = BDS_STOPSIGN;
		break;

	case BDS_REDLIGHT:
		if (bBrakingAlready)
		{
			bStopVal = 1;
			bVal = 0;
			bdsState = BDS_ALREADYBRAKING;
		}
		else
		{
			if (bNewPerceptionData)
			{
				if (nLightState == nPrevLightState && (fPrevConf >= MIN_LIGHT_CONF || fConf >= MIN_LIGHT_CONF) ||
					(nPrevLightState == LTST_RED && fPrevConf >= MIN_LIGHT_CONF && fConf < MIN_LIGHT_CONF) ||
					(nLightState == LTST_RED && fConf >= MIN_LIGHT_CONF && fPrevConf < MIN_LIGHT_CONF))
				{
					bdsState = BDS_REDLIGHT;
					nPrevLightState = nLightState;
					fPrevConf = fConf;
				}
				else
				{
					bdsState = BDS_SAME_INTERSECT;
					nPrevLightState = nLightState;
					fPrevConf = fConf;
					break;
				}


			} // end if New light state data
			if (fSpeed > SPEED_MIN_TO_BRAKE && fDist < fCalcStartToStopLoc || m_bBrakeDec)
			{
				bVal = 1;
				bStopVal = 1;
				if (nBrakeBrakeNodeId != nIntNodeId && nIntNodeId > 0)
				{
					PlayBrakeWarnAudio(2); // Braking alert
					nBrakeBrakeNodeId = nIntNodeId;
				}
			}
			else
			{
				bVal = 0;
				bStopVal = 1;
			}

			if ((fDist < fCalcStartToStopLoc + (fSpeed * nSecs)) && nBrakeWarnNodeId != nIntNodeId && nIntNodeId > 0)
			{
				bPlayBrakeWarn = true;
				PlayBrakeWarnAudio(1); // Warning alert
				nBrakeWarnNodeId = nIntNodeId;

			}


		}
		if (bNewPerceptionData)
		{
			nPrevLightState = nLightState;
			fPrevConf = fConf;
		}
		break;

	case BDS_STOPSIGN:
		if (bBrakingAlready)
		{
			bStopVal = 1;
			bVal = 0;  // don't decide to brake if someone is already applying the brake
			bdsState = BDS_ALREADYBRAKING;
		}
		else
		{
			// if we decided to brake before, then we should still be deciding to brake
			if (fSpeed > SPEED_MIN_TO_BRAKE && fDist < fCalcStartToStopLoc || m_bBrakeDec)
			{
				bVal = 1;
				bStopVal = 1;
				if (nBrakeBrakeNodeId != nIntNodeId && nIntNodeId > 0)
				{
					PlayBrakeWarnAudio(2); // Braking alert
					nBrakeBrakeNodeId = nIntNodeId;
				}
			}
			else
			{
				bVal = 0;
				bStopVal = 1;
			}

			if ((fDist < fCalcStartToStopLoc + (fSpeed * nSecs)) && nBrakeWarnNodeId != nIntNodeId &&
				nIntNodeId > 0 && !bPlayBrakeWarn)
			{
				bPlayBrakeWarn = true;
				PlayBrakeWarnAudio(1); // Warning alert
				nBrakeWarnNodeId = nIntNodeId;

			}

		}  // end else not already braking

		if (bNewPerceptionData)
		{
			nPrevLightState = nLightState;
			fPrevConf = fConf;
		}
		break;

	case BDS_YELLOW:
		fTimeYellow += TIME_LOOP_SEC;
		if (bNewPerceptionData)
		{
			if (nLightState == nPrevLightState && (fPrevConf >= MIN_LIGHT_CONF || fConf >= MIN_LIGHT_CONF) ||
				(nPrevLightState == LTST_YELLOW && fPrevConf >= MIN_LIGHT_CONF && fConf < MIN_LIGHT_CONF) ||
				(nLightState == LTST_YELLOW && fConf >= MIN_LIGHT_CONF && fPrevConf < MIN_LIGHT_CONF))
			{
				bdsState = BDS_YELLOW;

			}
			else
			{
				bdsState = BDS_SAME_INTERSECT;
			}
			nPrevLightState = nLightState;
			fPrevConf = fConf;

		} // if New light state data

		if (bdsState == BDS_YELLOW)
		{
			// Yellow light length is supposed to be between 3-6 seconds.  Longer for
			// higher speed limits.  Lets assume 3 for 25 mph (or 11 m/sec)
			// and 6 for 45 mph (or 20 m/sec)
			float fTimeToIntAtCurrentSpeed = fDist / fSpeed;
			float fTimeToRed = 0;
			if (fSpeed <= 11)
				fTimeToRed = 3;
			else if (fSpeed >= 20)
				fTimeToRed = 6;
			else
				fTimeToRed = ((fSpeed - 11) / 11.0 * 3) + 3;  // in seconds
			if (fTimeToIntAtCurrentSpeed > (fTimeToRed - fTimeYellow) || bPrevStopDecision)
			{
				bStopVal = 1;

				// we think we should be stopping at the intersection,
				// but is it time to brake yet?
				if (fSpeed > SPEED_MIN_TO_BRAKE && fDist < fCalcStartToStopLoc)
				{
					bVal = 1;
					if (nBrakeBrakeNodeId != nIntNodeId && nIntNodeId > 0)
					{
						PlayBrakeWarnAudio(2); // Braking alert
						nBrakeBrakeNodeId = nIntNodeId;
					}
				}
				if ((fDist < fCalcStartToStopLoc + (fSpeed * nSecs)) && nBrakeWarnNodeId != nIntNodeId &&
					nIntNodeId > 0 && !bPlayBrakeWarn)
				{
					bPlayBrakeWarn = true;
					PlayBrakeWarnAudio(1); // Warning alert
					nBrakeWarnNodeId = nIntNodeId;

				}

			}
			else  // we think we can get through on the yellow
			{
				bVal = 0;
				bStopVal = 0;
			}
		}

		break;

	case BDS_ALREADYBRAKING:
		bVal = 0;
		bStopVal = 1;

		break;

	case BDS_GREEN:
		if (bNewPerceptionData)
		{
			if (nLightState == nPrevLightState && (fPrevConf >= MIN_LIGHT_CONF || fConf >= MIN_LIGHT_CONF) ||
				(nPrevLightState == LTST_GREEN && fPrevConf >= MIN_LIGHT_CONF && fConf < MIN_LIGHT_CONF) ||
				(nLightState == LTST_GREEN && fConf >= MIN_LIGHT_CONF && fPrevConf < MIN_LIGHT_CONF))
			{
				bdsState = BDS_GREEN;
			}
			else
			{
				bdsState = BDS_SAME_INTERSECT;
			}
			nPrevLightState = nLightState;
			fPrevConf = fConf;

		} // if New light state data
		bVal = 0;
		bStopVal = 0;
		break;

	default:
		bdsState = BDS_NEWINTERSECT;
		break;

	}

	// This is the Yes or No to brake message
	char szMsg[8];
	memset(szMsg, 0, sizeof(szMsg));
	memcpy(szMsg, &bVal, 1);

	if (KvaserCard)
	{
		KvaserCard->SendMsg(1, lIdMsg, szMsg, 8, false);  //bus 0, only CAN we have connected.  
	}

	if (bNewGps)
	{

		fPrevSpeed = fSpeed;
		fPrevDist = fDist;
	}

	nPrevIntNodeId = nIntNodeId;
	PlayBrakeWarning->PutValue(bPlayBrakeWarn, fr);
	DecisionToStop->PutValue(bStopVal, fr);
	m_bBrakeDec = bVal;
	bPrevStopDecision = bStopVal;

	BrakeDecisionState->PutValue(bdsState, fr);

	return bVal;

}

*/