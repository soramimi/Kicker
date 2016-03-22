#include "Options.h"

Options::Options()
{
	undefined_shortcut_begavior = DelegateToTheDefaultShell;
}

Options::~Options()
{

}

#include "MySettings.h"

void Options::save()
{
	QString v;
	MySettings s;
	s.beginGroup("Behavior");
	if (getUndefinedShortcutBehavior() == DoNothing) {
		v = "nothing";
	} else {
		v = "delegate";
	}
	s.setValue("UndefinedShortcut", v);
	s.endGroup();
}

void Options::load()
{
	QString v;
	MySettings s;
	s.beginGroup("Behavior");
	v = s.value("UndefinedShortcut").toString();
	if (v == "delegate") {
		setUndefinedShortcutBehavior(DelegateToTheDefaultShell);
	} else {
		setUndefinedShortcutBehavior(DoNothing);
	}
	s.endGroup();
}
