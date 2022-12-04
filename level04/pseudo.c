/*
**  MAIN
*/

int __cdecl main(int argc, const char **argv, const char **envp)
{
  int stat_loc; // [esp+1Ch] [ebp-9Ch]
  char s; // [esp+20h] [ebp-98h]
  int v6; // [esp+A0h] [ebp-18h]
  int v7; // [esp+A4h] [ebp-14h]
  int v8; // [esp+A8h] [ebp-10h]
  int v9; // [esp+ACh] [ebp-Ch]

  v9 = fork();
  memset(&s, 0, 0x80u);
  v8 = 0;
  stat_loc = 0;
  if ( v9 )
  {
    do
    {
      wait(&stat_loc);
      v6 = stat_loc;
      if ( !(stat_loc & 0x7F) || (v7 = stat_loc, (char)((char)((stat_loc & 0x7F) + 1) >> 1) > 0) )
      {
        puts("child is exiting...");
        return 0;
      }
      v8 = ptrace(PTRACE_PEEKUSER, v9, 44, 0);
    }
    while ( v8 != 11 );
    puts("no exec() for you");
    kill(v9, 9);
  }
  else
  {
    prctl(1, 1);
    ptrace(0, 0, 0, 0);
    puts("Give me some shellcode, k");
    gets(&s);
  }
  return 0;
}