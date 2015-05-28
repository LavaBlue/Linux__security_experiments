/* What is this about?
 * This short code roughly illustrates what a potential attack means.
 * It is not intended as a 'virus'. However one can think of it in such a way.
 * Thereby I clearly state here that I cannot assume any responsibility upon 
 * what any person can do with this code.
 * (C) VladG, 2015 - All rights reserved.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>

typedef enum{FALSE, TRUE} BOOL;

int main()
{
	BOOL	bash_shell = FALSE, 
			sed_binary_installed = FALSE,
			usr_identified;	/* vlad: all these are about to be set up later */
	FILE	*fp = NULL;		/* vlad: about to be used in capturing the cmd output... */
	int		ret;			/* vlad: return value for system() call */
	char	ch,
			shellIdent[] = "echo $SHELL",
			sedIdent[] = "type sed", 
			usrIdent[] = "who|tr -s ' '|cut -d' ' -f1|head -1", 
			shellCmd1[] = "cat ~/.ssh/known_hosts | sed s/'ssh\\-rsa'/Attention\\!\\!\\!/",
			shellCmd2[] = "cat /etc/passwd|grep vlad",
			sedOutput[80],	/* vlad: use a text-based window length (usual 80 chars) for 'sed' binary */
			cmdOutput[80],	/* vlad: use a text-based window length (usual 80 chars) for the current shell */
			usrOutput[80];	/* vlad: use a text-based window length (usual 80 chars) for the current user */
	char	*pStr = NULL;
/* ==== */

	fprintf(stdout, "Starting identification...\n\n");
	/* vlad: check that I do have /bin/bash available (in order to run a specific command) 
	 * vlad: also check that you have 'sed' installed (there is a sed command that alters some text in a ssh-key)
	 */
	while(1)
	{
		/* vlad: find out the default shell (using an environment variable) */
		fp = popen(shellIdent, "r");
		if( fp != NULL)	/* vlad: execute shell command and capture its output. */
		{
			while( fgets(cmdOutput, sizeof(cmdOutput), fp) );	/* vlad: read one line at a time, since there is ONLY ONE line */
			fprintf(stdout, "Current shell: %s", cmdOutput);
			bash_shell = TRUE;
		} else {
			fprintf(stdout, "Something went wrong... Exiting now! Re-run your program later.\n");
			exit(1);
		}
		pclose(fp);	/* vlad: close the actual stream; if necessary, I can open another one later */

		/* vlad: check if 'sed' is installed */
		fp = popen(sedIdent, "r");
		if( fp != NULL)	/* vlad: execute shell command and capture its output. */
		{
			while( fgets(sedOutput, sizeof(sedOutput), fp) );	/* vlad: read one line at a time, since there is ONLY ONE line */
			if( strstr(sedOutput, "sed") != NULL )
			{
				pStr = strchr(sedOutput, '/');
				if( pStr != NULL )
					fprintf(stdout, "sed binary found at: %sBeware that by using 'sed', anyone can effectively change the file!\n", pStr);
				sed_binary_installed = TRUE;
			} else {
				fprintf(stdout, "sed NOT installed!");
				sed_binary_installed = FALSE;
			}
		} else {
			fprintf(stdout, "Something went wrong... Exiting now! Re-run your program later.\n");
			exit(1);
		}
		pclose(fp);	/* vlad: close the actual stream; if necessary I can open another one later */

		/* vlad: check for the actual user - and display it */
		fp = popen(usrIdent, "r");
		if( fp != NULL)	/* vlad: execute shell command and capture its output. */
		{
			while( fgets(usrOutput, sizeof(usrOutput), fp) );	/* vlad: read one line at a time, since there is ONLY ONE line */
			if( strlen(usrOutput) != 0 )	/* vlad: I do have a name identified */
			{
				fprintf(stdout, "Current user: %s\n", usrOutput);
				usr_identified = TRUE;
			} else {
				fprintf(stdout, "No current user?!");
				usr_identified = FALSE;
			}
		} else {
			fprintf(stdout, "Something went wrong... Exiting now! Re-run your program later.\n");
			exit(1);
		}
		pclose(fp);	/* vlad: close the actual stream; if necessary, I can open another one later */
		/* ==== */

		fprintf(stdout, "Do you really want to continue? (y/n)\n");
		system("/bin/stty raw");	/* vlad: avoid buffering the input; chars come directly from stdin. Not really necessary! */
		ch = getchar();
		system("/bin/stty cooked");	/* vlad: restore the terminal characteristics. Necessary if 'raw' was previously used. */
		if( ch == 'Y' || ch == 'y' )
		{
			/* vlad: the actual intervention, based on the pre-setup above: bash shell AND sed binary in place */
			if( bash_shell && sed_binary_installed && usr_identified)
			{
				if( system(NULL) )	/* vlad: double-check that the shell is available */
				{
					if((ret = system(shellCmd1)) != -1)	/* vlad: execute shell command. Did it fail? */
					{
						if ( WEXITSTATUS(ret) != 0 )	/* vlad: check the command's exit status */
						{
							fprintf(stdout, "Command failed! No such file(s) at indicated location\n");
							exit(1);	/* vlad: I cannot continue this way! */
						}
						
						if (	WIFSIGNALED(ret) &&		/* vlad: check for SIGINT and/or SIGQUIT signals possibly appearing while
														 * system() call runs 
														 */
								(WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
							break;
					}
				}
			}
		} else {
			fprintf(stdout, "No harm has been made to your station!\n");
		}
		fprintf(stdout, "\nDone!\n");
		break;
	}

	return 0;
}
