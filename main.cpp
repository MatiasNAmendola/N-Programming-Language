
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <cstring>
using namespace std;

	
//#define Windows	1
#define Linux	2
enum Error{ERROR_FATAL = -1, ERROR_COMMANDLINE, ERROR_FILENAME, 
		   ERROR_BADCOMMAND, ERROR_STARTSTRING, ERROR_ENDSTRING, 
		   ERROR_NOEOL, ERROR_EOLFOUND, ERROR_NOASSEMBLY};


bool    EndPause = false; 
FILE *  File;             
string  FileName;         
char *  Source;           
int     Size;            
int	    Position;         
int	    Line = 1;         
string  Command;         
string  AsmCode;          
string  AsmData;          
int	    DataCounter = 1;  
bool    Indent = false;   
bool    Build = true;     


void    RequestFile();    
void    ReadSource();     
void    StartAssembly();  
void    Compile();        
void    EndAssembly();    
void    BuildProgram();   
void    Error(int);       


string  GetCommand();     
string  GetString();      
string  GetNumber();      
void	SkipWhitespace(); 
void	GetEOL();        


void    WriteLine(string);
void    WriteData(string);
void    Cls();            
void    Rem();            
void    Print(string);    
void    Run(string);      
void    Sleep(string);    
void    Wait();           


string 	ToStr(int Number);
string  StripExt(string);


int main(int argc, char *argv[])
{

	if (argc < 2 || argc > 3)
	{
		RequestFile();
	}
	else
 	{

		FileName = argv[1];


		if (argc == 3)
		{
			string Switch = argv[2];


			if (Switch == "-compile")
				Build = false;
			else if (Switch == "-build")
				Build = true;
			else
				printf("Unknown switch: %s\n\n", argv[2]);
		}
	}


	if (Build && !EndPause)
	{

		printf("Welcome to Npl Compiler\n");
		printf("Written for N Programming Language ");
	}
	

	ReadSource();
	StartAssembly();
	Compile();
	EndAssembly();


	if (Build)
	{
		BuildProgram();


		printf("Your file compiler successfull.\n");
	}
	
 
	#ifdef Windows
		if (EndPause)
			system("pause");
	#endif

	return 0;
}


void RequestFile()
{

		printf("Welcome to Npl Compiler\n");
		printf("Written for N Programming Language\n");


	char Temp[1024];
	

	printf("Please enter the file to compile: ");
	
	
	scanf("%s", &Temp);
	FileName = Temp;


	printf ("\n");
	

	EndPause = true;
}


void ReadSource()
{

	string BasicFile = FileName;
	File = fopen(BasicFile.c_str(), "rb");


	if (!File)
		Error(ERROR_FILENAME);
	

	fseek(File, 0, SEEK_END);
	Size = ftell(File);
	rewind(File);
	

	Size = Size - ftell(File) + 1;
	

	Source = new char[Size];
	

	memset(Source, '\n', Size);
	

	fread(Source, Size - 1, 1, File);
	fclose(File);


	FileName = StripExt(FileName);
}


void StartAssembly()
{

	WriteLine("EXTERN	system");
	WriteLine("EXTERN	puts");
	WriteLine("EXTERN	getchar");
	WriteLine("EXTERN 	exit");
	

	#ifdef Windows
		WriteLine("EXTERN	_sleep");
	#endif
	#ifdef Linux
		WriteLine("EXTERN	sleep");
	#endif
	

	WriteLine("");
	

	WriteData("section .data");
	WriteData("");
	

	#ifdef Windows
		WriteData("ClearScreen 		db \"cls\",0");
	#endif
	#ifdef Linux
		WriteData("ClearScreen 		db \"clear\",0");
	#endif
	

	WriteLine("section .text");
	WriteLine("");
	
	#ifdef Windows
		WriteLine("global	START");
		WriteLine("START:	");
	#endif
	#ifdef Linux
		WriteLine("global	_start");
		WriteLine("_start:	");
	#endif


	Indent = true;
	
	WriteLine("PUSHA	");
	WriteLine("PUSH	EBP");
	WriteLine("MOV	EBP, ESP");
}


void Compile()
{
	
	while (Position < Size)
	{
		
		while (Position < Size && isspace(Source[Position]))
			Position++;
			
		
		Command = GetCommand();
		
		
		if (Command == "")
		{
			
			++Position;
			continue;
		}
		else if (Source[Position]=='#')
		{
			Rem();
		}
		else if (Command == "CLS")
		{
			
			Cls();
		}
		else if (Command == "PRINT")
		{
			
			Print(GetString());
		}
		else if (Command == "RUN")
		{
			
			Run(GetString());
		}
		else if (Command == "SLEEP")
		{
			
			Sleep(GetNumber());
		}
		else if (Command == "WAIT")
		{
			
			Wait();
		}
		else
		{
		
			Error(ERROR_BADCOMMAND);
		}
		

		GetEOL();
	}
	

	delete[] Source;
}


void EndAssembly()
{

	string AssemblyFile = FileName + ".asm";
	File = fopen(AssemblyFile.c_str(), "w");
	

	if (!File)
		Error(ERROR_NOASSEMBLY);
	
	Print("Press ENTER to exit...");
	Wait();
	

	WriteLine("MOV	ESP, EBP");
	WriteLine("POP	EBP");
	WriteLine("POPA	");
	WriteLine("PUSH	0");
	WriteLine("CALL	exit");
	WriteLine("");
	WriteLine("");
	

	fwrite(AsmCode.c_str(), AsmCode.length(), 1, File);
	fwrite(AsmData.c_str(), AsmData.length(), 1, File);
	fclose(File);
}


void BuildProgram()
{
	#ifdef Windows

		string Command = "nasm -fwin32 \"" +FileName+ ".asm\"";
		system(Command.c_str());
		

		Command = "nlink /CONSOLE /ni \"" +FileName + ".obj\"";
		Command = Command + " msvcrt.dll";
		system(Command.c_str());
	#endif
	#ifdef Linux

		string Command = "nasm -felf \"" + FileName + ".asm\"";
		system(Command.c_str());
		

		Command = "ld -s -dynamic-linker /lib/ld-linux.a -o ";
		Command+= "\"" + FileName + "\" \"" + FileName + ".o\"";
		Command+= " /lib/libc.a";
		system(Command.c_str());
	#endif
	
	return;
}


void Error(int Code)
{

	string Message = "Error on line " + ToStr(Line) + ": ";
	

	switch (Code)
	{
		case ERROR_COMMANDLINE:
			printf("Error! Bad command line.\n");
			printf("Usage: nc <filename> [-compile -build]");
			break;
		case ERROR_FILENAME:
			Message = "Error! File \"" + FileName;
			Message = Message + "\" does not exist.";
			printf(Message.c_str());
			break;
		case ERROR_BADCOMMAND:
			printf(Message.c_str());
			Message = "Unknown command: " + Command;
			printf(Message.c_str());
			break;
		case ERROR_STARTSTRING:
			printf(Message.c_str());
			printf("Expected start of a string.");
			break;
		case ERROR_ENDSTRING:
			printf(Message.c_str());
			printf("String does not end.");
			break;
		case ERROR_NOEOL:
			printf(Message.c_str());
			printf("End of Line expected.");
			break;
		case ERROR_EOLFOUND:
			printf(Message.c_str());
			printf("Command not finished before EOL.");
			break;
		case ERROR_NOASSEMBLY:
			printf("Error! Could not open assembly file.");
			break;
		default:
			printf("Error! Something really bad happened!");
	}
	
	printf("\n");
	

	#ifdef Windows
		if (EndPause)
			system("pause");
	#endif
	
	exit(1);
}


string GetCommand()
{

	string Command = "";
	SkipWhitespace();
	

	while (Position < Size && isalpha(Source[Position]))
	{
		Command += toupper(Source[Position]);
		++Position;
	}
	

	return Command;
}


string GetString()
{
	string String = "";
	

	SkipWhitespace();


	if (Source[Position] != '"')
		Error(ERROR_STARTSTRING);


	++Position;


	while (Source[Position] != '"')
	{

		if (Position > Size || Source[Position] == '\n')
			Error(ERROR_ENDSTRING);

		String += Source[Position];
		++Position;
	}
	

	++Position;

	return String;
}


string GetNumber()
{
	string Number = "";


	SkipWhitespace();


	while (Position < Size && isdigit(Source[Position]))
	{
		Number += Source[Position];
		++Position;
	}
	
	return Number;
}


void GetEOL()
{

	bool FoundEOL = false;


	while (Position < Size && isspace(Source[Position]))
	{

		if (Source[Position] == '\n')
		{
			FoundEOL = true;
			Line++;
		}
		
		++Position;
	}
	

	if (FoundEOL == false)
		Error(ERROR_NOEOL);
}


void SkipWhitespace()
{

	while (isspace(Source[Position]))
	{

		if ((Source[Position] == '\n' || Position > Size))
			Error(ERROR_EOLFOUND);
			
		++Position;
	}
}


void WriteLine(string AsmLine)
{

	if (Indent == true)
		AsmCode += "\t" + AsmLine + "\n";
	else
		AsmCode += AsmLine + "\n";
	return;
}


void WriteData(string AsmLine)
{
	AsmData += AsmLine + "\n";
}


void Cls()
{
	WriteLine("PUSH	ClearScreen");
	WriteLine("CALL	system");
	WriteLine("POP	EAX");
}


void Print(string Line)
{

	string Name = "String_" + ToStr(DataCounter);
	DataCounter++;
	

	WriteData(Name + " 		db \"" + Line + "\",0");
	

	WriteLine("PUSH	" + Name);
	WriteLine("CALL	puts");
	WriteLine("POP	EAX");
}

void Run(string Command)
{

	string Name = "String_" + ToStr(DataCounter);
	DataCounter++;
	

	WriteData(Name + " 		db \"" + Command + "\",0");
	

	WriteLine("PUSH	" + Name);
	WriteLine("CALL	system");
	WriteLine("POP	EAX");
}


void Sleep(string Seconds)
{

	#ifdef Windows
		WriteLine("MOV	EAX," + Seconds);
		WriteLine("MOV	EDX,1000");
		WriteLine("MUL	EDX");
		WriteLine("PUSH	EAX");
	#endif
	#ifdef Linux
		WriteLine("PUSH	" + Seconds);
	#endif
	

	#ifdef Windows
		WriteLine("CALL	_sleep");
	#endif
	#ifdef Linux
		WriteLine("CALL	sleep");
	#endif
	

	WriteLine("POP	EAX");
}


void Wait()
{
	WriteLine("CALL	getchar");
}


void Rem()
{
	while (Position < Size && Source[Position] != '\n')
		++Position;
}


string ToStr(int Number){
	string Result;
	char Temp[128];
	
	sprintf(Temp, "%i", Number);
	
	Result = Temp;
	return Result;
}


string StripExt(string FileName)
{
	unsigned int Dot = FileName.rfind(".");

	if (Dot != string::npos)
		return FileName.substr(0, Dot);

	return FileName;
}

