#ifndef __IO_OBJECT_H__
#define __IO_OBJECT_H__

#include <iostream>
#include <Yail.h>

namespace Yail {

YAIL_INTERFACE(IoService);

YAIL_BEGIN_CLASS_EXT(IoObject, EXTENDS(YObject))
  void __new__IoObject() { fd_ = -1; }
  void __del__IoObject();
  public:
    void init(String name, int fd, SPtr<IoService> ioSvc, bool isBlocking=false);
    void deinit();

    friend Ostream& operator<<(Ostream& os, const SelfType& obj);

    String name() { return name_; }
    int fd() { return fd_; }

    virtual void handleIo(bool rdReady, bool wrReady) {
      assert("Must be implemented in sub-classes" && false);
    }

    void pauseRx() { rxReq(false, "pause rx"); }
    void requestRx() { rxReq(true, "request rx"); }
    void resumeRx() { rxReq(true, "resume rx"); }
    void resetRx() { rxReq(false, "reset rx"); }
    bool rxPaused() { return rxPaused_; }

    void pauseTx() { txReq(true, "pause tx"); } 
    void requestTx() { txReq(true, "request tx"); } 
    void resumeTx() { txReq(false, "resume tx"); } 
    void resetTx() { txReq(false, "reset tx"); } 
    void idleTx() { txReq(false, "idle tx"); }
    bool txPaused() { return !txReady_; }
    bool txReady()  { return txReady_; }

    unsigned long long rxCnt() { return rxCnt_; }
    unsigned long long txCnt() { return txCnt_; }
    void incRxCnt(int cnt=1) { rxCnt_ += cnt; }
    void incTxCnt(int cnt=1) { txCnt_ += cnt; }

  protected:
    SPtr<IoService> ioSvc() { return ioSvc_; }

  private:
    String name_;
    int fd_;

    bool rxPaused_;
    bool txReady_;
    unsigned long long rxCnt_;
    unsigned long long txCnt_;

    SPtr<IoService> ioSvc_;

    void rxReq(bool onOff, String msgStr);
    void txReq(bool onOff, String msgStr);
YAIL_END_CLASS

}

#endif

