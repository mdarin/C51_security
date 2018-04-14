#include <stdio.h>
#include <string.h>


/*
int main(void) {
	char str[] = " - This, a sample strig.";
	printf("Splittig strig:\n%sinto tokens:\n, str);
	char* pch = strtok(str, " \r\n");
	while (NULL != pch) {
		printf("%s\n", pch);
		pch = strtok(NULL, " \r\n");
	}
	return 0;
}
*/

int get_status(char* at_response) {
	int status = 0;
	if (NULL != at_response) {
		char* token = strtok(at_response, " \n\r");
		while (NULL != token) {
			//printf("[%s]\n", token);
			if (0 == strcmp("ERROR", token)) {
				status = 1;
			}
			token = strtok(NULL, " \r\n");
		}
	}
	return status;
}


int main(void) {
	char *ok = "+CMGM HELLO WORLD\r\nOK\r\n";
	char *error = "ERROR\r\n";
	printf("%s\n", ok);
	printf("%s\n", error);
	char* token = strtok(ok, " \n\r");
	while (NULL != token) {
		printf("[%s]\n", token);
		token = strtok(NULL, " \r\n");
	}

	token = strtok(error, " \n\r");
	while (NULL != token) {
		printf("[%s]\n", token);
		token = strtok(NULL, " \r\n");
	}

	int status = get_status(ok);
	printf ("stat: %d\n", status);
	status = get_status(error);
	printf ("stat: %d\n", status);

	return 0;
}
