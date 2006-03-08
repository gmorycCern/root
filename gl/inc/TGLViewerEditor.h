#ifndef ROOT_TGLViewerEditor
#define ROOT_TGLViewerEditor

#include <memory>

#ifndef ROOT_TGedFrame
#include "TGedFrame.h"
#endif

#ifndef ROOT_TGLUtil
#include "TGLUtil.h"
#endif

class TGCheckButton;
class TGNumberEntry;
class TGButtonGroup;
class TGroupFrame;
class TGRadioButton;
class TGTabElement;
class TGButton;
class TGLViewer;
class TGTab;

class TGLViewerEditor : public TGedFrame {
private:
   //Pointers to manipulate with tabs
   TGTabElement     *fGuidesTabEl;
   TGTabElement     *fClipTabEl;
   TGCompositeFrame *fGuidesFrame;
   TGCompositeFrame *fClipFrame;
   //"Lights" tab's controls 
   TGGroupFrame     *fLightFrame;
   TGButton         *fTopLight;
   TGButton         *fRightLight;
   TGButton         *fBottomLight;
   TGButton         *fLeftLight;
   TGButton         *fFrontLight;
   //"Guides" tab's controls
   TGButtonGroup    *fAxesContainer;
   TGRadioButton    *fAxesNone;
   TGRadioButton    *fAxesEdge;
   TGRadioButton    *fAxesOrigin;
   TGGroupFrame     *fRefContainer;

   TGCheckButton    *fReferenceOn;
   TGNumberEntry    *fReferencePosX;
   TGNumberEntry    *fReferencePosY;
   TGNumberEntry    *fReferencePosZ;

   //"Cplipping" tab's controls
   EClipType         fCurrentClip;
   TGButtonGroup    *fTypeButtons;

   TGCompositeFrame *fPlanePropFrame;
   TGNumberEntry    *fPlaneProp[4];

   TGCompositeFrame *fBoxPropFrame;
   TGNumberEntry    *fBoxProp[6];
   TGCheckButton    *fEdit;
   TGButton         *fApplyButton;
   //Model
   TGLViewer        *fViewer;

   void ConnectSignals2Slots();
   
   TGLViewerEditor(const TGLViewerEditor &);
   TGLViewerEditor &operator = (const TGLViewerEditor &);

   void CreateLightsTab();
   void CreateGuidesTab();
   void CreateClippingTab();
   
public:
   TGLViewerEditor(const TGWindow *p, Int_t id,
                   Int_t width = 140, Int_t height = 30,
                   UInt_t options = kChildFrame,
                   Pixel_t back = GetDefaultFrameBackground());
                   
   ~TGLViewerEditor();

   void SetModel(TVirtualPad *pad, TObject *obj, Int_t event);
   void SetGuides();
   void SetCurrentClip();
   //Lights manipulation
   void DoButton();
   //Axis manipulation
   void UpdateViewerGuides();
   void UpdateReferencePos();
   //Clipping manipulation
   void ClipValueChanged();
   void ClipTypeChanged(Int_t);
   void UpdateViewerClip();
   
   ClassDef(TGLViewerEditor, 0) //GUI for editing TGLViewer attributes
};

#endif
