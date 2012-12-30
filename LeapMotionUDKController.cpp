// LeapMotionUDKController.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Leap.h"
#include <stdio.h>
#include <iostream>

#define _USE_MATH_DEFINES // To get definition of M_PI
#include <math.h>
#include <map>

#define MAX_HANDS 4
#define MAX_FINGERS 5

#define LEFT_HAND 0
#define RIGHT_HAND 1

#define HAND_TIMESTAMP_TIMEOUT 5000000

using namespace std;

extern "C"
{
	struct FingerPosition
	{
		float posX;
		float posY;
		float posZ;
		int id;
	};

	struct PalmPosition
	{
		float posX;
		float posY;
		float posZ;
	};

	struct PalmRotation
	{
		float pitch;
		float yaw;
		float roll;
	};

	struct LeapMotionPalm
	{
		float radius;
		struct PalmRotation rotation;
		struct PalmPosition position;
		int id;
	};

	struct LeapMotionHand
	{
		struct LeapMotionPalm palm;
		std::map<int , struct FingerPosition> fingerPositions;
		int fingerCount;
		int64_t time_stamp;
	};

	class LeapListener : public Leap::Listener 
	{
	  public:
			LeapListener();
			virtual void onInit(const Leap::Controller&);
			virtual void onConnect(const Leap::Controller&);
			virtual void onDisconnect(const Leap::Controller&);
			virtual void onFrame(const Leap::Controller&);

			bool IsConnected();

			LeapMotionHand *GetHand(int handIdx);
			FingerPosition *GetFinger(int handIdx, int fingerIdx);
		private:
			struct LeapMotionHand* AddHand(int newId);
			struct FingerPosition* AddFinger(struct LeapMotionHand* hand, int newId);
		public:
			std::map<int, struct LeapMotionHand> mHandList;
			bool mConnected;
	};

	LeapListener::LeapListener(void)
	{
		mConnected = false;
	}

	void LeapListener::onInit(const Leap::Controller& controller) 
	{
	  //std::cout << "Initialized 1:21" << std::endl;
	}

	void LeapListener::onConnect(const Leap::Controller& controller) 
	{
	  //std::cout << "Connected" << std::endl;
		mConnected = true;
	}

	void LeapListener::onDisconnect(const Leap::Controller& controller) 
	{
		mConnected = false;
		//std::cout << "Disconnected" << std::endl;
	}

	struct LeapMotionHand* LeapListener::AddHand(int newId)
	{
		struct LeapMotionHand newHand;
		std::map<int, struct LeapMotionHand>::iterator iter;

		iter = mHandList.begin();
		mHandList.insert(iter, pair<int, struct LeapMotionHand>(newId, newHand));

		return &(mHandList.find(newId)->second);
	}

	struct FingerPosition *LeapListener::AddFinger(struct LeapMotionHand* hand, int newId)
	{
		struct FingerPosition newFinger;
		std::map<int, struct FingerPosition>::iterator iter;

		iter = hand->fingerPositions.begin();
		hand->fingerPositions.insert(iter, pair<int, struct FingerPosition>(newId, newFinger));

		return &(hand->fingerPositions.find(newId)->second);
	}

	//STORE ALL THE HAND DATA HERE
	void LeapListener::onFrame(const Leap::Controller& controller) 
	{
		const Leap::Frame frame = controller.frame();
		const std::vector<Leap::Hand>& hands = frame.hands();
		const size_t numHands = hands.size();
		size_t handCounter;

		if (numHands >= 1) 
		{
			for(handCounter = 0; handCounter < hands.size(); ++handCounter)
			{
				struct LeapMotionHand *curHand = NULL;
				if(mHandList.find(hands.at(handCounter).id()) != mHandList.end())
				{
					//hand is found
					curHand = &(mHandList.find(hands.at(handCounter).id())->second);
				}
				else
				{
					//new hand id, create
					curHand = AddHand(hands.at(handCounter).id());
				}

				const Leap::Hand& hand = hands.at(handCounter);
				const std::vector<Leap::Finger>& fingers = hand.fingers();
				const size_t numFingers = fingers.size();

				curHand->time_stamp = frame.timestamp();

				if(numFingers)
				{
					for(size_t i = 0; i < fingers.size(); ++i)
					{
						struct FingerPosition *curFinger;
						if(curHand->fingerPositions.find(fingers.at(i).id()) != curHand->fingerPositions.end())
						{
							curFinger = &(curHand->fingerPositions.find(fingers.at(i).id())->second);
						}
						else
						{
							//add finger
							curFinger = AddFinger(curHand, fingers.at(i).id());
						}

						const Leap::Finger& finger = fingers.at(i);
						const Leap::Ray& tip = finger.tip();

						curFinger->id = finger.id();
						curFinger->posX = (float) tip.position.x;
						curFinger->posY = (float) tip.position.y;
						curFinger->posZ = (float) tip.position.z;
					}
				}

				const Leap::Ray* palmRay = hand.palm();
				
				if (palmRay != NULL) 
				{
					const Leap::Vector palm = palmRay->position;
					const Leap::Vector wrist = palmRay->direction;
			
					PalmPosition palmPos;
					palmPos.posX = (float) palm.x;
					palmPos.posY = (float) palm.y;
					palmPos.posZ = (float) palm.z;

					curHand->palm.position = palmPos;

					// Check if the hand has a normal vector
					const Leap::Vector* normal = hand.normal();
					if (normal != NULL) 
					{
						// Calculate the hand's pitch, roll, and yaw angles
						double pitchAngle = -atan2(normal->z, normal->y) * 180/M_PI + 180;
						double rollAngle = -atan2(normal->x, normal->y) * 180/M_PI + 180;
						double yawAngle = atan2(wrist.z, wrist.x) * 180/M_PI - 90;
					
						// Ensure the angles are between -180 and +180 degrees
						pitchAngle = (pitchAngle > 180) ? (pitchAngle - 360) : pitchAngle;
						rollAngle = (rollAngle > 180) ? (rollAngle - 360) : rollAngle;
						yawAngle = (yawAngle > 180) ? (yawAngle - 360) : yawAngle;

						PalmRotation palmRotation;
						palmRotation.pitch = (float) pitchAngle;
						palmRotation.roll = (float) rollAngle;
						palmRotation.yaw = (float) yawAngle;

						curHand->palm.rotation = palmRotation;						
					}
				}
			}
		}	

		//clear hands that have timed out
		if(mHandList.size())
		{
			map<int, struct LeapMotionHand>::iterator iter;
			for(iter = mHandList.begin(); iter != mHandList.end(); ++iter)
			{
				if( (frame.timestamp() - (*iter).second.time_stamp) > HAND_TIMESTAMP_TIMEOUT)
				{
					mHandList.erase(iter);
					break;
				}
			}
		}
	}

	LeapMotionHand *LeapListener::GetHand(int handIdx)
	{
		//hand Idx will be either left or right hand
		map<int, struct LeapMotionHand>::iterator iter;

		if(!mHandList.size())
		{
			return NULL;
		}

		struct LeapMotionHand* curHand = NULL;

		for(iter = mHandList.begin(); iter != mHandList.end(); ++iter)
		{
			//boundary for safe values
			if ((*iter).second.palm.position.posX < -500 || (*iter).second.palm.position.posX > 500)
			{
				continue;
			}

			if(!curHand)
			{
				curHand = &(*iter).second;
			}

			//check the position depending on the hand type the user wants
			if(handIdx == LEFT_HAND)
			{
				if((*iter).second.palm.position.posX > curHand->palm.position.posX)
				{
					curHand = &((*iter).second);
				}
			}
			else 
			{
				if((*iter).second.palm.position.posX < curHand->palm.position.posX)
				{
					curHand = &((*iter).second);
				}
			}
		}

		return curHand;
	}

	struct FingerPosition *LeapListener::GetFinger(int handIdx, int fingerIdx)
	{
		struct LeapMotionHand *curHand = GetHand(handIdx);
		if(curHand)
		{
			return &(curHand->fingerPositions.begin()->second);
		}
		return NULL;
	}


	bool LeapListener::IsConnected()
	{
		return mConnected;
	}
	
	/**************************************
	******udk controller class************
	**************************************/

	class LeapUDKController
	{
		public:
			void Initialize();
			int GetNumFingers(int handIdx);
			void GetFingerPositions(int handIdx, int fingerIdx, float &posX, float &posY, float &posZ);
			void GetPalmPosition(int handIdx, float &posX, float &posY, float &posZ);
			void GetPalmRotation(int handIdx, float &pitch, float &yaw, float &roll);
			float GetHandBallRadius(int handIdx);
			bool IsConnected();
			//singleton
			static LeapUDKController* GetInstance();
		public:
			static LeapUDKController* mInstance;
			static Leap::Controller *mLeapController;
			LeapListener mListener;
	};

	LeapUDKController* LeapUDKController::mInstance;
	Leap::Controller*  LeapUDKController::mLeapController;

	LeapUDKController *LeapUDKController::GetInstance()
	{
		if(!mInstance)
		{
			mInstance = new LeapUDKController;
		}

		return mInstance;
	}

	void LeapUDKController::Initialize()
	{
		if(!mInstance)
		{
			mInstance = new LeapUDKController;
		}

		if(!mLeapController)
		{
			mLeapController = new Leap::Controller(&(mInstance->mListener));
		}
	}

	int LeapUDKController::GetNumFingers(int handIdx)
	{
		return mInstance->mListener.GetHand(handIdx)->fingerCount;
	}
	
	void LeapUDKController::GetFingerPositions(int handIdx, int fingerIdx, float &posX, float &posY, float &posZ) 
	{
		if(mInstance->mListener.GetFinger(handIdx, fingerIdx))
		{
			posX = mInstance->mListener.GetFinger(handIdx, fingerIdx)->posX;
			posY = mInstance->mListener.GetFinger(handIdx, fingerIdx)->posY;
			posZ = mInstance->mListener.GetFinger(handIdx, fingerIdx)->posZ;		
		}
		else
		{
			posX = posY = posZ = -1;
		}
	}	
	
	void LeapUDKController::GetPalmPosition(int handIdx, float &posX, float &posY, float &posZ) 
	{
		if(mInstance->mListener.GetHand(handIdx))
		{
			posX = mInstance->mListener.GetHand(handIdx)->palm.position.posX;
			posY = mInstance->mListener.GetHand(handIdx)->palm.position.posY;
			posZ = mInstance->mListener.GetHand(handIdx)->palm.position.posZ;
		}
		else
		{
			std::cout << "Position: Hand not found, idx : " << handIdx << " total available : " << mInstance->mListener.mHandList.size() << endl;
			posX = posY = posZ = -1;
		}
	}	
	
	void LeapUDKController::GetPalmRotation(int handIdx, float &pitch, float &yaw, float &roll) 
	{
		if(mInstance->mListener.GetHand(handIdx))
		{
			pitch = mInstance->mListener.GetHand(handIdx)->palm.rotation.pitch;
			yaw = mInstance->mListener.GetHand(handIdx)->palm.rotation.yaw;
			roll = mInstance->mListener.GetHand(handIdx)->palm.rotation.roll;		
		}
		else
		{
			std::cout << "Rotation: Hand not found, idx : " << handIdx << " total available : " << mInstance->mListener.mHandList.size() << endl;
			pitch = yaw = roll = -1;
		}
	}	
	
	float LeapUDKController::GetHandBallRadius(int handIdx)
	{
		return mInstance->mListener.GetHand(handIdx)->palm.radius;
	}

	bool LeapUDKController::IsConnected()
	{
		return mInstance->mListener.IsConnected();
	}


	/**************************
	***DLL EXPORTED CALLS******
	**************************/

	__declspec(dllexport) void InitLeapMotion()
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			instance->Initialize();
		}
	}

	
	__declspec(dllexport) int GetNumFingers(int handIdx)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->GetNumFingers(handIdx);
		}

		return -1;
	}

	__declspec(dllexport) void GetFingerPositions(int handIdx, int fingerIdx, float &posX, float &posY, float &posZ)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->GetFingerPositions(handIdx, fingerIdx, posX, posY, posZ);
		}
	}

	__declspec(dllexport) void GetPalmPosition(int handIdx, float &posX, float &posY, float &posZ)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->GetPalmPosition(handIdx, posX, posY, posZ);
		}
	}

	__declspec(dllexport) void GetPalmRotation(int handIdx, float &pitch, float &yaw, float &roll)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->GetPalmRotation(handIdx, pitch, yaw, roll);
		}
	}
	
	__declspec(dllexport) float GetHandBallRadius(int handIdx)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->GetHandBallRadius(handIdx);
		}

		return -1.0f;
	}

	__declspec(dllexport) bool IsLeapMotionConnected(int handIdx)
	{
		LeapUDKController *instance = LeapUDKController::GetInstance();

		if(instance)
		{
			return instance->IsConnected();
		}

		return false;
	}
}
