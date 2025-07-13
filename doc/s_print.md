# Printer routines

Cross-platform printer support is provided (in SymbOS 4.0 and up) by the system print daemon. There are two standard ways of sending a print job to the printer: directly via the print daemon, or via the PrintIt helper program. On CPC and MSX machines only, it is also possible to directly send data to the printer port, using functions found in `symbos/device.h`.

## Contents

* [Printing via the daemon](#printing-via-the-daemon)
* [Printing via PrintIt](#printing-via-printit)
* [Direct printer functions](#direct-printer-functions)

## Printing via the daemon

When running, the printer daemon (`PRINTD.EXE`) periodically scans the subfolder `PRINTD` within the system directory for files with the extension `.JOB`. Once a `.JOB` file is found, it is sent to the printer, with the daemon handling all the necessary hardware management. Printing a file therefore just requires creating a file in the `PRINTD` subfolder:

```c
char fid;
char filepath[256];

// create output filename
strcpy(filepath, Sys_Path());
strcat(filepath, "PRINTD\\PR742A.TMP");

// open file
fid = File_New(_symbank, filepath);
if (fid <= 7) {
    // ... opened correctly, write to file ...
	// ... write code goes here ...
	// ... then close and rename to .JOB
	File_Close(fid);
	Dir_Rename(_symbank, filepath, "PR742A.JOB");
}
```

As this example demonstrates, it is good practice to initially write the file with a different extension and only rename it to `.JOB` when finished; otherwise, there is a possibility that the printer daemon may start trying to print the file before it is finished being written. Files sent to the printer in this way should be in plain ASCII, without extended characters (ASCII value >127), and we are responsible for word-wrapping any lines over 80 characters. Standardized formatting codes for bold, italic, etc. can be included, which the printer daemon will automatically convert to the correct formatting for the user's printer; see `README.TXT` in the printer daemon folder for the most up-to-date information on supported formatting codes.

## Printing via PrintIt

Directly sending a job to the print daemon is flexible, but requires us to perform all necessary formatting, including word-wrapping and pagination (splitting the file into pages with the correct spacing for the user's printer). This can be very fiddly, so SymbOS provides a standardized formatting program, PrintIt (`PRINTIT.EXE`), which can always be found in the printer daemon folder. To use PrintIt, we simply pass the full path of an ASCII text file to `PRINTIT.EXE` as a command-line option. PrintIt will bring up a settings window with various formatting options; margins, word-wrap, line numbering, etc. When the user is satisfied with their settings, they just click "Print" and PrintIt automatically formats the file and sends it to the printer daemon.

For normal ASCII printing, using PrintIt is highly recommended because it lets the user set and remember the correct page-formatting settings for their printer across multiple apps. (We can still include formatting codes, which will be passed through unmodified to the printer daemon; see above.)

```c
char fid;
char printcmd[256];
char filepath[256];

// create temporary file PRINT.TMP in the same folder as our app
Dir_PathAdd(0, "PRINT.TMP", filepath);
fid = File_New(_symbank, filepath);
if (fid <= 7) {
    // ... opened correctly, write to file ...
	// ... write code goes here ...
	File_Close(fid);
	
	// create PrintIt command (e.g., %PRINTD\PRINTIT.EXE C:\APPDIR\PRINT.TMP);
	// note that App_Run() understands "%" as the system folder.
	strcpy(printcmd, "%PRINTD\\PRINTIT.EXE ");
	strcat(printcmd, filepath);
	
	// run it
	App_Run(_symbank, printcmd, 0);
}
```

## Direct printer functions

In addition to `symbos.h`, these functions can be found in `symbos/device.h`.

### Print_Busy()

*Currently only available in development builds of SCC.*

```c
signed char Print_Busy(void);
```

On CPC and MSX machines only, checks the status of the connected parallel printer. (This function interfaces directly with the printer hardware rather than going through the daemon, so it should only be used when direct hardware control is required. For normal printing, use PrintIt or the printer daemon.)

*Return value*: 0 = printer is ready to receive data; -1 = printer is busy; 1 = platform unsupported.

### Print_Char()

*Currently only available in development builds of SCC.*

```c
signed char Print_Char(unsigned char ch);
```

On CPC and MSX machines only, sends the single character `ch` to the connected parallel printer. (This function interfaces directly with the printer hardware rather than going through the daemon, so it should only be used when direct hardware control is required. For normal printing, use PrintIt or the printer daemon.)

Note that, on the Amstrad CPC, the high bit of `ch` will be ignored because the CPC printer port only connects 7 of the 8 data lines. Normal ASCII (<128) should work, but extended ASCII (>127) or 8-bit binary data may require workarounds.

*Return value*: 0 = printed successfully; 1 = platform unsupported; 2 = timeout.

### Print_String()

*Currently only available in development builds of SCC.*

```c
signed char Print_String(char* str);
```

On CPC and MSX machines only, sends the ASCII string `str` the connected parallel printer. (This function interfaces directly with the printer hardware rather than going through the daemon, so it should only be used when direct hardware control is required. For normal printing, use PrintIt or the printer daemon.)

*Return value*: 0 = printed successfully; 1 = platform unsupported; 2 = timeout.
