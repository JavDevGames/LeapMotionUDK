/***********************************
******LEAP MOTION UDK INTEGRATION***
***********************************/

Hello, this is Javier from JavDevGames.

To use the code in this repository you're going to need the following things:

1. The latest version of UDK
2. LeapMotion SDK
3. LeapMotion DevBoard
4. Visual Studio

Once you have UDK and LeapMotion installed, you can do the following things:

1. Copy the DLL in this repository to:
  -UDK\UDK-xxxx-xx\Binaries\Win32\UserCode
2. Copy the Leap.dll that comes with the LeapMotion SDK to:
	-UDK\UDK-xxxx-xx\Binaries\Win32\UserCode
3. Copy the UnrealScript file (LeapMotionUDKDLL.uc) in this repository to:
	-UDK\UDK-xxxx-xx\Development\Src\[Your_Script_Folder]


Then somewhere in your the Unreal Script classes you've created, create an instance of the LeapMotionUDKDLL class by doing one of these:

var LeapMotionUDKDLL mMotionController;
mMotionController = new (none) class'LeapMotionUDKDLL';
mMotionController.InitLeapConnection();

Don't forget to call InitLeapConnection, otherwise you won't get any frame updates.

Last updated 31/12/2012.
