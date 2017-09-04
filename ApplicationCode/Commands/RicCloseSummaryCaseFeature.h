#pragma once
#include "cafCmdFeature.h"
#include "vector"

class RimSummaryCase;

class RicCloseSummaryCaseFeature : public caf::CmdFeature
{
	CAF_CMD_HEADER_INIT;
public:

protected:
	// Overrides
	virtual bool isCommandEnabled();
	virtual void onActionTriggered(bool isChecked);
	virtual void setupActionLook(QAction* actionToSetup);
private:
	RimSummaryCase* selectedSummaryCase() const;
};

