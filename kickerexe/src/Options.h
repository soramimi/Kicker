#ifndef OPTIONS_H
#define OPTIONS_H


class Options {
public:
	enum UndefinedShortcutBehavior {
		DoNothing,
		DelegateToTheDefaultShell,
	};

private:
	UndefinedShortcutBehavior undefined_shortcut_begavior;
public:
	Options();
	~Options();

	void save();
	void load();

	void setUndefinedShortcutBehavior(UndefinedShortcutBehavior t)
	{
		undefined_shortcut_begavior = t;
	}

	UndefinedShortcutBehavior getUndefinedShortcutBehavior()
	{
		return undefined_shortcut_begavior;
	}
};

extern Options *the_options;

#endif // OPTIONS_H
