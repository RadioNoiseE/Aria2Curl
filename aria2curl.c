/* Copyright 2023, Jing Huang. (Under MIT license) */

# include <stdio.h>
# include <string.h>
# include <uchar.h>

# define MAXTOK 102400
# define MAXTL 102410
# define MAXKOPT 1024
# define MAXFLN 42
# define LITRADT "# Parsed (generated) by aria2curl.\n\
                  # Copyright, 2023 Jing Huang.\n\
                  # https://github.com/RadioNoiseE/Aria2Curl (MIT license).\n"

fpos_t current_pos;
int terminate_flag = 0;

int file_init(FILE * input)
{
  if (fgetpos(input, &current_pos)) {
    printf("File position initialisation failed [ERR]\n");
    return -1;
  }
  return 0;
}

struct {
  char token_list[MAXTOK];
  int type;
} line;

char psd_token_list[MAXTL];

enum {
  url,
  key_opt,
  comment,
  unknown
};

int read_line(FILE * input)
{
  if (fsetpos(input, &current_pos)) {
    printf("File position setup failed [ERR]\n");
    return -1;
  }

  char c;
  int cnt;

  for (cnt = 0; (c = getc(input)) != '\n' && c != EOF && cnt < MAXTOK; cnt++)
    line.token_list[cnt] = c;
  line.token_list[cnt] = '\0';

  if (c == '\n')
    fgetpos(input, &current_pos);
  else if (c == EOF)
    terminate_flag = 1;
  else {
    printf("A logic error has occurred, seems like the file has changed during processing [ERR]\n");
    return -1;
  }

  return 0;
}

int classify_line(void)
{
  int trial, temp;

  for (trial = 0; line.token_list[trial] == ' ' || line.token_list[trial] == '\t'; trial++);
  
  if (line.token_list[trial] == '#') {
    line.type = comment;
    return 0;
  }

  while (trial >= 0) {
    temp = trial;
    
    if (trial == (MAXKOPT - 2)) {
      printf("Unknown magic, this line doesn't fall into any of the categories and hence will be ignored [WRN]\n");
      line.type = unknown;
      return 0;
    } else if (line.token_list[trial] == '=') {
      line.type = key_opt;
      return 0;
    } else if (line.token_list[temp++] == ':' && line.token_list[temp++] == '/' && line.token_list[temp] == '/') {
      line.type = url;
      return 0;
    } else
      trial++;
  }

  return -1;
}

void parse_url(int pb_flag)
{
  strcpy(psd_token_list, "url=\"");
  strncat(psd_token_list, line.token_list, 1018);
  strcat(psd_token_list, "\"\n");
  if (pb_flag)
    strcat(psd_token_list, "progress-bar\n");
  return;
}

void parse_kopt(void)
{
  int offset, offset_aux, cnt, cnt_aux;
  char temp[MAXKOPT], tmp[MAXKOPT];

  for (offset = 0; line.token_list[offset] != '='; offset++)
    temp[offset] = line.token_list[offset];
  temp[offset] = '\0';

  for (cnt = 0; temp[cnt] == ' ' || temp[cnt] == '\t'; cnt++);
  for (cnt_aux = 0; temp[cnt] != '\0'; cnt++, cnt_aux++)
    tmp[cnt_aux] = temp[cnt];
  tmp[cnt_aux] = '\0';

  if (strcmp(tmp, "out") == 0)
    strcpy(tmp, "output");
  strcpy(psd_token_list, tmp);
  strcat(psd_token_list, "=\"");
  offset++;

  for (offset_aux = 0; line.token_list[offset] != '\0'; offset++, offset_aux++)
    temp[offset_aux] = line.token_list[offset];
  temp[offset_aux] = '\0';
  
  strcat(psd_token_list, temp);
  strcat(psd_token_list, "\"\n");
  return;
}

void parse_comment(void)
{
  strcpy(psd_token_list, line.token_list);
  strcat(psd_token_list, "\n");
  return;
}

void parse_unknown(void)
{
  strcpy(psd_token_list, "");
  return;
}

int write_line(FILE * output)
{
  fputs(psd_token_list, output);
  return 0;
}

int main(int argc, char * argv[])
{
  int output_default = 1;
  int progress_bar = 0;
  char option, output_fname[MAXFLN];
  FILE *ifp, *ofp;

  while (--argc > 0 && (*++argv)[0] == '-') {
    option = *++argv[0];

    switch (option) {
      case '#':
        progress_bar = 1;
        break;
      case 'o':
        output_default = 0;
        break;
      default:
        printf("__FILE__: illegal option %c\n", option);
        argc = 0;
        break;
    }
  }

  if (argc != (output_default ? 1 : 2)) {
    printf("aria2curl: feeds aria2c input file to curl\n");
    printf("usage    : aria2curl [-o output|-#] input\n");
    return -1;
  } else {
    if (output_default)
      strcat(strcpy(output_fname, *argv), ".cfg");
    else
      strcpy(output_fname, *argv++);

    ofp = fopen(output_fname, "w");
    ifp = fopen(*argv, "r");

    file_init(ifp);
    fputs(LITRADT, ofp);
    
    while (terminate_flag == 0) {
      read_line(ifp);
      classify_line();

      if (line.type == url)
        parse_url(progress_bar);
      else if (line.type == key_opt)
        parse_kopt();
      else if (line.type == comment)
        parse_comment();
      else if (line.type == unknown)
        parse_unknown();
      else
        printf("Unknown magic, how come there's a line that doesn't belong to any of the categories [FTL]\n");

      write_line(ofp);
    }
    
    if (fclose(ofp) != 0)
      printf("Error in closing file %s [ERR]\n", output_fname);

    printf("=> %s parsed, output written to %s\n", *argv, output_fname);

    return 0;
  }
}
