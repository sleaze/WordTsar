# WordTsar

Wordstar for the 21st Century. WordTsar is a Wordstar 7.0D document mode clone. It loads Wordstar 4, Wordstar 7, RTF (partial), and DOCX(partial) files, and saves in Wordstar 7 and RTF format.

WordTsar is currently Alpha. What does Alpha mean? Alpha means the program works, but is feature incomplete. Please do not send reports on non-implemented features. Feel free to discuss them in the forum though.

(2022-03-26) This is version **0.3.409 pre-Alpha** *(see below)* for Windows, Linux and MacOS.

__See the Announcemnets under Discussion for details__

The Linux builds were compiled on Ubuntu 18.04 and are an executable AppImage https://appimage.org/. Simply unzip and run.  
The MacOS build is compiled on Catalina and is dynamically linked with QT using QtCreator.  
The Windows build is compiled on Windows 10 and is dynamically linked with QT using Visual Studio 2019.  

There have been a fair amount of core changes to this version. If you use WordTsar on a daily basis, make sure you keep the old version around... just in case.

- Fixed Ticket #46 __(Regression)__ "Save" should "Save as" if the file has not been saved before
- Fixed Ticket #47 WordTsar seems to be unable to print only one line of text.
- Fixed Ticket #49 ^KD and ^KQ not working correctly on edited document
- Fixed Ticket #50 backup file not path-joined
- Fixed Ticket #52 Deleting pages does not lower page count in the status bar
- Fixed Ticket #53 File Save quietly fails

__NOTES__

- 0.3.409 has not been extensively tested on MacOS. Linux and Windows use has been medium.
- I am personally putting it into daily use (eat your own dog food). 
- I haven’t had a crash lately, but it can still happen. BE CAREFUL! 
- A backup of your file is made every 1 minute (not when opening DOCX files). 
- The initial page/paper size is 8.5" x 11"
- Download the implemented.xlsx file to see what’s done and what’s next.
- the 0.3.x releases use UTF8 for all in-memory storage of the document. This required some large changes in the backend (dropping the gapbuffer document storage for a paragraph based storage system, as well as a ton of changes in layout and document management. That's why this is labeled **pre-alpha** instead of alpha.
 
*__NOTE:__ for people submitting support/bugs. Please include only one bug/issue per ticket. Multiple issues per ticket is a pain to manage for me. Windows Users: unzip the file and read the readme.txt.*

## You can follow us on Twitter at #wordtsar or on Facebook @wordtsar

__Source code GNU Affero General Public License v3.0.__