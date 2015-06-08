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

#include<assert.h>
#include<signal.h>
#include<unistd.h>

typedef enum{FALSE, TRUE} BOOL;

int main()
{
	BOOL	bash_shell = FALSE, 
			sed_binary_installed = FALSE,
			usr_identified,
			usr_home_identified,
			user_home_changes = FALSE;	/* vlad: all these are about to be set up later */
	FILE	*fp = NULL;		/* vlad: about to be used in capturing the cmd output... */
	int		ret;			/* vlad: return value for system() call */
	char	ch,
			strHome[80] = "/home/",
			shellIdent[] = "echo $SHELL",
			sedIdent[] = "type sed", 
			usrIdent[] = "who|tr -s ' '|cut -d' ' -f1|head -1", 
			usrHomeIdent[] = "cat /etc/passwd|grep vlad",
			/* *pHomeIdent = NULL, */
			shellCmd1[] = "cat ~/.ssh/known_hosts | sed s/'ssh\\-rsa'/Attention\\!\\!\\!/",
			shellCmd2[] = "cat /etc/passwd|grep vlad|sed s/home//",
			sedOutput[80],		/* vlad: use a text-based window length (usual 80 chars) for 'sed' binary */
			cmdOutput[80],		/* vlad: use a text-based window length (usual 80 chars) for the current shell */
			usrOutput[80],		/* vlad: use a text-based window length (usual 80 chars) for the current user */
			usrHomeOutput[80];	/* vlad: use a text-based window length (usual 80 chars) for the current user's home dir */
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
				{
					fprintf(stdout, "sed binary found at: %sBeware that by using 'sed', anyone can effectively change the file!\n", pStr);
					sed_binary_installed = TRUE;
					pStr = NULL;	/* vlad: reset this pointer for furhter use */
				} else {
					fprintf(stdout, "sed NOT installed!");
					sed_binary_installed = FALSE;
				}
			} else {
				fprintf(stdout, "Something went wrong... Exiting now! Re-run your program later.\n");
				exit(1);
			}
		}
		pclose(fp);	/* vlad: close the actual stream; if necessary I can open another one later */

		/* vlad: check for the actual user - and display it */
		fp = popen(usrIdent, "r");
		if( fp != NULL)	/* vlad: execute shell command and capture its output. */
		{
			while( fgets(usrOutput, sizeof(usrOutput), fp) );	/* vlad: read one line at a time, since there is ONLY ONE line */
			if( strlen(usrOutput) != 0 )	/* vlad: I do have a name identified */
			{
				fprintf(stdout, "Current user: %s", usrOutput);
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

		/* vlad: identify the current user's home dir and display it */
		fp = popen(usrHomeIdent, "r");
		if( fp != NULL )
		{
			while( fgets(usrHomeOutput, sizeof(usrHomeOutput), fp) );	/* vlad: read one line (80 chars) at a time */
			if( strlen(usrHomeOutput) != 0 )	/* vlad: I do have a string here? */
			{
				fprintf(stdout, "cat /etc/passwd: %s", usrHomeOutput);
				char *pStrToHome = strcat(strHome, usrOutput);	/* vlad: now I have the current user's home dir */
				if( pStrToHome != NULL ) usr_home_identified = TRUE;	/* vlad: and now I can alter it */
				fprintf(stdout, "Current user's home dir: %s\n", pStrToHome);
#if 0
				pStr = strstr(usrHomeOutput, pStrToHome);
				pStr[strlen(pStr)] = '\0';
				fputs(pStr, stdout);
				if( pStr != NULL)
				{
					usr_home_identified = TRUE;
					fprintf(stdout, "Current user's home dir: %s\n", pStr);
				} else {
					/* vlad: TODO: user perror()? It conforms to C89, C99 and POSIX.1-2001 */
					fprintf(stdout, "Can't have current user's home dir... (!)\n");	
				}
#endif
			} else {
				fprintf(stdout, "No current user?!");
				usr_home_identified = FALSE;
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
			/* vlad: First attack: change the known_host file; it is based on the pre-setup above: bash shell AND sed binary in place */
			if( bash_shell && sed_binary_installed && usr_identified)
			{
				fprintf(stdout, "First attempt: change SSH known_hosts file...\n");
				if( system(NULL) )	/* vlad: double-check that the shell is available */
				{
					if((ret = system(shellCmd1)) != -1)	/* vlad: execute shell command. Did it fail? */
					{
						if ( WEXITSTATUS(ret) != 0 )	/* vlad: check the command's exit status */
						{
							fprintf(stdout, "Command failed! No such file(s) at indicated location\n");
							user_home_changes = TRUE;	/* vlad: known_hosts file is absent => try to modify the current user's home dir*/
						}

						/* vlad: check for SIGINT and/or SIGQUIT signals possibly appearing while system() call runs */
						if ( WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT) )
							break;	/* vlad: TODO: what do I have to do here? */

					}	/* vlad: system(shellCmd1)) */
				}	/* vlad: if (system(NULL)) */
			}	/* vlad: if ( bash_shell && sed_binary_installed && usr_identified ) */

			/* vlad: Second attack, if the first one didn't work: change the current user's home dir (but not physically!) */
			puts("");
			user_home_changes = TRUE;	/* vlad: this shouldn't be explicitly put here, unless the first attack fails */
			assert((user_home_changes == TRUE ) && ( usr_home_identified == TRUE));
			if(( user_home_changes == TRUE ) && ( usr_home_identified == TRUE))
			{
				fprintf(stdout, "Second attempt: alter the current user's home dir...\n");
				if((ret = system(shellCmd2)) != -1)	/* vlad: execute shell command. Did it fail? */
				{
					if ( WEXITSTATUS(ret) != 0 )	/* vlad: if it failed, then check the command's exit status */
					{
						/* vlad: TODO: user perror()? It conforms to C89, C99 and POSIX.1-2001 */
						fprintf(stdout, "Command failed!\n");
						exit(1);	/* vlad: I cannot continue! */
					}

					/* vlad: check for SIGINT and/or SIGQUIT signals possibly appearing while system() call runs */
					if ( WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
						break;	/* vlad: TODO: what do I have to do here? */

				}	/* vlad: system(shellCmd2)) */
			}	/* vlad: if( user_home_changes && usr_home_identified ) */
		} else {
			fprintf(stdout, "Ending transparently!\n");
			return 0;
		}

		fprintf(stdout, "Be calm: NO actual harm has been made to your station!\n");
		fprintf(stdout, "\nDone!\n");
		break;
	}

	return 0;
}

#if 0
		/* vlad: read one line at a time, since there is ONLY ONE line*/
		while( fgets(user_home_changes, sizeof(user_home_changes), fp) ); 
#endif
