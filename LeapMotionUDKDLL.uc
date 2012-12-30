class LeapMotionUDKDLL extends Object
	DLLBind(LeapMotionUDKController);

dllimport final function InitLeapMotion();

dllimport final function int GetNumFingers(int handIdx);
dllimport final function GetFingerPositions(int handIdx, int fingerIdx, out float posX, out float posY, out float posZ);

dllimport final function GetPalmPosition(int handIdx, out float posX, out float posY, out float posZ);
dllimport final function GetPalmRotation(int handIdx, out float pitch, out float yaw, out float roll);
dllimport final function float GetHandBallRadius(int handIdx);
dllimport final function bool IsLeapMotionConnected();

function InitLeapConnection()
{
	InitLeapMotion();
}

function int LeapMotion_GetFingerCount(int handIdx)
{
	local int numFingers;

	numFingers = GetNumFingers(handIdx);

	//`log("Number of hands : " @ numFingers); 

	return numFingers;
}

function LeapMotion_GetFingerPosition(int handIdx, int fingerIdx)
{
	local float x;
	local float y;
	local float z;

	GetFingerPositions(handIdx, fingerIdx, x, y, z);

	`log("Finger positions : " @ x @ y @ z); 
}

function LeapMotion_GetPalmPosition(int handIdx, out float x, out float y, out float z)
{
	GetPalmPosition(handIdx, x, y, z);
}

function LeapMotion_GetPalmRotation(int handIdx, out float pitch, out float yaw, out float roll)
{
	GetPalmRotation(handIdx, pitch, yaw, roll);
	
	//`log("Palm rotation :  " @ pitch @ yaw @ roll);
}

function bool LeapMotion_IsConnected()
{
	return IsLeapMotionConnected();
}