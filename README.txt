Horizon 0.8.6

////////////////

Key Shortcuts:

F1 to F10:
 - Programmable QuickChannels

F11:
 - Connect or Reconnect (if already connected) current profile

F12:
 - Disconnect current profile

Numpad 0-9:
 - Switches active bot profile to cooresponding number, and sets focus to the textbox.

Ctrl + Enter:
 - Converts the textbox to multilined mode if isn't already, and adds a newline.

Ctrl + J:
 - Toggles join/leave notifications.

Ctrl + L:
 - Toggles lock chat. No text will be added to the current profile's RichEdit.

Ctrl + B:
 - Toggles buffered chat. Text will be stored in a buffer until this is toggled off, where it adds the stored text.

Enter + End:
 - Insta-send. Bypasses queue and sends raw 0x0e packet.

-----------------------------------------------------------------

Greet/Idle Message Variables:
 %gu Joined username
 %gp Joined ping
 %gf Joined flags

 %us System uptime
 %uc Connection uptime
 %ul Loaded uptime
 %b  Bancount
 %n  Users in channel
 %p  Current ping
 %f  Current flags
 %c  current channel
 %s  Total packets sent
 %r  Total packets received
 %j  Time last user joined in seconds
 %o  Amount of users that joined the channel
 %a  # of chats received
 %w  Current winamp song
 %%  The character %

-----------------------------------------------------------------

Postmortem Debugging Minidumps
On exception the custom ExceptionInfo top level exception handler is called, which creates a minidump file specified in the message box. This can be sent for later analysis.

-----------------------------------------------------------------

Also... beware of the 'Show Columns' Listview option ;) it's a Windows bug with listview column header windows when the parent window has WS_EX_COMPOSITED extended style

------------------------------------------------------------------

 - CHANGELOG - 

 Version 0.7.2.4
  - First version to be used on a casual basis


 Version 0.7.2.8
  - Added three new commands
    - /crash, to force an access violation exception
    - /internalvars, to view internal-use only variables
    - /setcwd, to set the current working directory if directory is moved


 Version 0.7.3.2
  - Removed the connected status identifier of the Status Bar
  - Moved 'client' assignment on 0x0A parse instead of channel join
  - Added the current client icon to the Client part of status bar
  - Fixed plug bug
  - Scrolls down on rtb chop
  - Slightly expanded Client status bar part
  - Made icons transparent


 Version 0.7.4.0
  - Greatly simplified textbox system (now fully works too!)
  - added whisper hotkey
  - Fixed ping icons
  - Redid splash screen with DrawText() instead of ExtTextOut()


 Version 0.7.4.4
  - Fixed Diablo 2 asterisk chatting accelerator support
  - Added favorite channels treeview, with custom draw
  - Added handling for account closures
  - Shows clan in tooltip if needed
  - Added Quick switch


 Version 0.7.5.0
  - Added solid border to Quick switch draw rect
  - Pressing tab with no ctrl sets focus to current textbox
  - Added a mask for config tab control icons
  - Added channel hotkeys (f1-f10)
  - Removed auto detecting scroll lock, manually enabled/disabled
  - Config code cleaned up


 Version 0.7.5.6
  - Fixed fatal bug where the timer is never killed on queue clear
  - Added color selection for gradients
  - Added combobox to select tab control position
  - Added gradient option
  - Changed interface config GUI


 Version 0.7.6.3
  - Added selection of brush hatch types for gdi fill
  - Made Atmosphere system optional
  - Extended brushes to ListView ToolTips
  - Removed unused Pen GDI object
  - Fixed font change RichEdit selection issue
  - Doesn't create new font etc if old font face name/size is same
  - Added parsing for WCG players and D2 characters


 Version 0.7.7.0
  - Made QuickSwitch an alpha layered window (for now)
  - Now downloads CR dll files and extracts
  - Revamped configuration
  - Window rect not saved if maximized
  - Fixed ping icon insertion on flags update
  - Added fade effect on close
  - When unloading a profile, doesn't destroy channel imagelist
  - Fixed config GDI leaks: config color brushes, font preview, tab imagelist


 Version 0.7.7.2
  - No longer attempts sending 0x51 on failed checkrevision
  - MPQ extraction fixed - set the filesize to 0 on accident


 Version 0.7.7.4
  - Fixed diablo 2 asterisk removal
  - Fixed open folder menu item


 Version 0.7.8.2
  - Disconnects profile on CR failure
  - Clears connection specific flags on disconnect
  - Cross-profile proofed cr lib dl complete message
  - Sends SID_NULL (0x00) every minute
  - Slightly modified bnftp progress text, and truncated decimals
  - Fixed richedit menu about item
  - Fixed profile switch on CTRL key up
  - Added support to change tab position (with tab styles)


 Version 0.7.8.7
  - Tab style changes based on position
  - Button row follows
  - Fixed font selection on config
  - Added tray notifications for dropping and flooding
  - Atmosphere reloads on configuration save


 Version 0.7.9.0
  - Client flags now set on connect
  - Added /activeping command, checks current ad too
  - Now use bswap instruction for 32 bit reversals
  - Variable length textbox send buffer, max line count 32
  
 Version 0.7.9.3
  - Misc bug fixes
  - Friends/Clan listview tab added
  - Added warden module downloading, checking, preparation, loading, and execution.

 Version 0.8.0.0
  - Fixed warden 0x02 check (variable packet IDs)
  - Fixed join-game message specific \n sanity check byte overwriting
  - If >= 223 characters sent, broken into another line
  - Fixed bug with sending malloc() heap padding (0xCDs), whoops :p
  - Sets textbox back color dark red if over character limit
  - Fixed logs on clearing, now writes 0x0A with 0x0D
  - Fixed problem with pasting multiple lines, if it's queued the AddChat() isn't called,
    therfore the first nullbyte isn't written over with 0x0A, used to append the text, therefore terminates.

 Version 0.8.0.5
  - Friends/Clan listview tab perfected, along with internal channel list chain.
  - Fixed double free() called under rare circumstances in EID_FLAGSUPDATE
  - Refactored ping spoof to service window messages while waiting

 Version 0.8.2.0
  - Compiled with MSVC9!!!! haha disregard that, i sux cox
  - Changed underlying profile backbone - MAJOR!
  - Added status window
  - Config text for sample font now updates on the size textbox's focus kill
  - Misc profile load/unload bug fixes
  - Double buffered about dialog, improved color fade algorithm

 Version 0.8.2.6
  - Fixed tab icon sync issues
  - Fixed window repositioning load

 Version 0.8.2.7
  - Fixed menu sync issues, connect checkmark

 Version 0.8.3.0
  - Fixed some fatal problems with chain functions, referencing bad mem
  - Added error handling for unloadable dlls
  - Cleaned up chat d2 character handling

 Version 0.8.4.0
  - Saved position of status window
  - adds misc. login information to status window
  - Made interface a bit smoother
  - Movable tabs

 Version 0.8.4.4
  - Made warden able to be parsed by any client
  - Fixed malformed 0x52 packet sent when creating SRP logon accounts
  - Fixed memory leak when creating SRP logon accounts
  - Fixed profile mismatch with resize on load

 Version 0.8.4.9
  - Fixed imagelist destruction on profile unload
  - Frees BNCSUTIL NLS handle on profile disconnect
  - Fixed removal of menu item on profile unload

 Version 0.8.6.0
  - Added except handler to AddLog, fputs very occasionally crashes due to an error on MSVCRT's part
  - clicking Windows->profile menu items switches to that profile's window, doesn't connect
  - Cleaned up 0x02 parsing code
  - Fixed accidentally reversed 0x75 clan tag (whoops)
  - Fixed clan/friend listview tooltips
  - Added code to unload modules used for warden 0x02 on shutdown

 Version 0.8.8.0
  - Changed listview caption on tab change (friends, clan)
  - WSAAsyncSelect() now sends MDI children winsock events
  - Fixed small memory leak when sending internal commands
  - Fixed idle timer, reduced to one per profile.
  - Added display of flood count when flooding was finished
  - Added null to clan name replacement str in channellist when reconstructed
  - Fixed wc3 channel join self-omission from channel list
  - Duplicate chat packet EID_USERs for just-connected WC3 users now updates
  - Added support for friends list packets
  - Expanded Warcraft 3 clan support
  - Added a Warcraft 3 clan creator
  - Fixed ungraceful disconnect tab image
  - Cleaned command parsing code
  - Made initial winsock connection asynchronous
  - Added auto reconnect
  - Make xrencrypt autodecode
  - make xrencrypt channel/factor user specified
  - Added query command, removed internalvars/sent/received/bancount
  - Added clearchat command
  - Volatile profile attributes now preserved on config reload, whoops :(
  - Fixed "Time Logged" attribute parsing
  - Added BNI icon file downloading and parsing
  - Added "Export BNI to BMP" function
  - Added "Glass" dekstop image alphablending GUI background feature
  - Modularized BNFTP downloading and synchronization methods

//  - Fix resizing of RTB on scrollbar creation
//  - Do something about the addprofile/deleteprofile
//  - Fix current RTB logging code
//  - Add logging on chat clear
//  - Make game code more complete
//  - Added tab to space converter on text send
//  - add fun sentence morphers
//  - Proxy support added
//  - Add config code for new options
