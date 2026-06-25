#include "DobotArm.h"

// Dobot Magician SDK (lib/DobotSDK). The SDK transmits on Serial1 internally.
#include "Protocol.h"     // ProtocolInit(), ProtocolProcess(), gSerialProtocolHandler
#include "command.h"      // PTPCmd / PTPCommonParams / PTPJumpParams + SetPTP* API
#include "RingBuffer.h"   // RingBufferIsFull(), RingBufferEnqueue()
#include "Message.h"      // Message, MessageWrite(), MessageRead(), MessageProcess()
#include "ProtocolID.h"   // ProtocolGetPose
#include <string.h>       // memset(), memcpy()

static PTPCmd          gPTPCmd;            // working point-to-point command
static PTPCommonParams gPTPCommonParams;   // velocity / acceleration ratios
static PTPJumpParams   gPTPJumpParams;     // JUMP-mode lift height
static uint64_t        gQueuedCmdIndex;    // queue bookkeeping (required by the API)

// Drain Serial1 into the SDK's RX ring buffer so it can parse the arm's replies.
static void dobotSerialRead() {
  while (Serial1.available()) {
    uint8_t data = Serial1.read();
    if (RingBufferIsFull(&gSerialProtocolHandler.rxRawByteQueue) == false) {
      RingBufferEnqueue(&gSerialProtocolHandler.rxRawByteQueue, &data);
    }
  }
}

void dobotProcess() {
  dobotSerialRead();
  ProtocolProcess();
}

void dobotInit() {
  Serial1.begin(115200);
  ProtocolInit();

  gPTPCommonParams.velocityRatio     = 100;   // 0..100 % of max speed
  gPTPCommonParams.accelerationRatio = 100;
  SetPTPCommonParams(&gPTPCommonParams, true, &gQueuedCmdIndex);

  gPTPJumpParams.jumpHeight    = 20;   // lift height used in JUMP mode (mm)
  gPTPJumpParams.maxJumpHeight = 100;  // highest Z it may jump to (mm)
  SetPTPJumpParams(&gPTPJumpParams, true, &gQueuedCmdIndex);

  dobotProcess();   // flush the parameter commands to the arm
}

void goTo(float x, float y, float z, float r) {
  gPTPCmd.ptpMode = JUMP_XYZ;   // lift-across-lower: best for pick & place
  gPTPCmd.x = x;
  gPTPCmd.y = y;
  gPTPCmd.z = z;
  gPTPCmd.r = r;
  SetPTPCmd(&gPTPCmd, true, &gQueuedCmdIndex);
  dobotProcess();
}

bool dobotGetPose(float &x, float &y, float &z, float &r) {
  // 1. Send a GetPose READ request (rw = 0, no params).
  Message req;
  memset(&req, 0, sizeof(req));
  req.id       = ProtocolGetPose;   // 10
  req.rw       = 0;                 // read
  req.isQueued = 0;
  req.paramsLen = 0;
  MessageWrite(&gSerialProtocolHandler, &req);
  ProtocolProcess();   // serialise + flush the request out on Serial1

  // 2. Wait for the reply. The Pose payload is x,y,z,r,j1..j4 (8 floats); we
  //    only need the first four (the Cartesian pose, mm / deg).
  unsigned long start = millis();
  while (millis() - start < 500) {
    dobotSerialRead();                          // Serial1 -> rx ring buffer
    MessageProcess(&gSerialProtocolHandler);    // parse raw bytes -> rx packets
    Message resp;
    if (MessageRead(&gSerialProtocolHandler, &resp) == ProtocolNoError) {
      if (resp.id == ProtocolGetPose && resp.paramsLen >= 16) {
        memcpy(&x, &resp.params[0],  4);
        memcpy(&y, &resp.params[4],  4);
        memcpy(&z, &resp.params[8],  4);
        memcpy(&r, &resp.params[12], 4);
        return true;
      }
    }
  }
  return false;   // no reply -> check Serial1 wiring / arm power
}
