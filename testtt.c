void
consoleintr(int (*getc)(void))
{
  int c;
  int i;
  if (execvim == 1)
  {
     acquire(&input.lock);
     while((c = getc()) >= 0){
	int filePos;
	if (c != 0)
	{
	  outb(CRTPORT, 14);
	  filePos = inb(CRTPORT+1) << 8;
	  outb(CRTPORT, 15);
	  filePos |= inb(CRTPORT+1);
	
	  if (firstVim)
	  {
	    endPos =filePos;
      if (endPos % 80 !=0)
        endPos =filePos +80 - filePos % 80;
      for (startPos = filePos - filePos % 80; checkVimStart(startPos); startPos --);
      if (startPos < 0)
        startPos = 0;
      else
        startPos += 80 - startPos % 80;
      if (startPos == endPos)
      {
        int i;
        for (i= startPos; i<startPos+80; i++)
          crt[i] = (' '&0xff) | 0x0700;
        endPos += 80;
      }
      firstVim = 0;
      filePos = startPos;
	  }
  }

  if (c != 0x1B && c!= 0)
  {
	  switch (c){
        //up
	  case 0xe2:
      if (filePos >= startPos + 80)
        filePos -= 80;
            break;

	//down
    case 0xe3:
	  if (filePos < endPos - 80)
		filePos += 80;
          break;

	//left
        case 0xe4:
	  if (filePos > startPos )
		filePos--;
          break;
	
	//right
	case 0xe5:
	  if (filePos < endPos-1 )
		filePos++;
          break;
	
	case '\n':
	  if (endPos == 23*80 )
	  {
	    memmove(crt, crt+80, sizeof(crt[0])*(filePos/80)*80);
	    int i;
	    for (i = (filePos/80)*80; i< (filePos/80)*80 +80; i++)
		crt[i] = (' '&0xff) | 0x700;
	    startPos -= 80;
	  }
	  else 
	  {
	    filePos += 80 - filePos%80;
	    memmove(crt+filePos+80, crt+filePos, sizeof(crt[0])*(endPos-filePos));
	    int i;
            for (i = filePos; i<filePos+80; i++)
		crt[i] = (' '&0xff) | 0x700;
            endPos += 80; 
	  }
	  filePos -= filePos%80; 
          break;
	case C('H'):
	  if (filePos > startPos)
	  {
	    int i;
            int rmline = 1;

		if (crt[filePos] != ((' '&0xff) | 0x0700))
		{
		  rmline = 0;

		}
	    if (crt[filePos]  == ((' '&0xff) | 0x0700) && filePos % 80 ==0  && rmline)
	    {
		memmove(crt+filePos, crt+filePos+80, sizeof(crt[0])*(endPos-filePos-80));
		for (i = endPos -80; i< endPos; i++)
		  crt[i] = (' '&0xff) | 0x0700;
		endPos -= 80;
		while (crt[--filePos] == ((' '&0xff) | 0x0700) && filePos %80 != 0);
		if (crt[filePos] != ((' '&0xff) | 0x0700))
		  filePos ++;
	    }
	    else
	    {
		crt[--filePos] = ((' '&0xff) | 0x0700);
                for (i = filePos; i< endPos; i++)
		{
		  if (i % 80 ==79 && crt[i] == ((' '&0xff) | 0x0700))
			break;
		  crt[i] = crt[i+1];
		}
		if (filePos % 80 == 79){
		  while (crt[--filePos] == ((' '&0xff) | 0x0700) && filePos % 80 !=0);
		  if (filePos % 80 == 0 && crt[filePos] == ((' '&0xff) | 0x0700))
		  {
		    memmove(crt+filePos, crt+filePos+80, sizeof(crt[0])*(endPos-filePos-80));
		    for (i= endPos -80; i < endPos; i++)
			crt[i] = ((' '&0xff) | 0x0700);
		    endPos -= 80;
		  }
		  else
		    filePos++;
		}
	    }
          }
	  break;
	
	default:
	  i = filePos + 79 - filePos % 80;
	  while (crt[i] != ((' '&0xff) | 0x0700))
	    i += 80;
          if (i < endPos)
	  {
	    for (; i > filePos; i--)
              crt[i] = crt[i-1];
	  }
	  else
          {
	    i = filePos + 80 - filePos % 80;
	    memmove(crt+i+80, crt+i, sizeof(crt[0])*(endPos-i));
	    int j;
	    for (j = i; j < i + 80; j++)
		crt[j] = (' '&0xff) | 0x0700;
	    for (; i > filePos; i--)
		crt[i] = crt[i-1];
	    endPos += 80;
	  }
	  crt[filePos ++] = (c&0xff) | 0x0700;
	  break;
	}
	if ((endPos/80) >= 24){
	  memmove(crt, crt+80, sizeof(crt[0])*80*23);
	  startPos -= 80;
	  endPos -= 80;
	  filePos -= 80;
	  int i;
	  for (i = endPos; i< endPos +80; i++)
	    crt[i] = (' '&0xff) | 0x0700;
	}
	
	outb(CRTPORT, 14);
	outb(CRTPORT+1, filePos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, filePos);

	}

	else if (c != 0)
	{
	  firstVim = 1;
	  int i ,j;
	  memset(content, 0, sizeof(char)*2000);
	  for (i = startPos, j=0; i< endPos; i++)
	  {
	    if (crt[i] != ((' '&0xff) | 0x0700))
	    {
		content[j++] = crt[i] & 0x00ff;
	    }
	    else
	    {
	      int k;
	      for (k= i+1; (k%80) != 0 && crt[k] == ((' '&0xff) | 0x0700); k++);
	      if (k % 80 == 0)
	      {
		content[j++] = '\n';
		i = k - 1;
	      }
	      else
	      {
		for (; crt[i] == ((' '&0xff) | 0x0700); i++)
		  content[j++] = ' ';
		i--;
	      }
	    }
	  }
	    content[j++] = 0;
	    outb(CRTPORT, 14);
	    outb(CRTPORT+1, endPos >> 8);
	    outb(CRTPORT, 15);
	    outb(CRTPORT+1, endPos);
	    execvim = 0;
	}
    }
    release(&input.lock);
    return;	
  }
  else 
  {
  acquire(&input.lock);
  char commondPrefix[INPUT_BUF];
  int length;
  while((c = getc()) >= 0){
    if(c != '\t' && c != 0) {tab_times = 0;}
    switch(c){
    case C('P'):  // Process listing.
      his.flag = 0;//history flag
      bufferPos = 0;
      procdump();
      break;
    case '\t':
      bufferPos = 0;
      his.flag = 0;//history flag
      if (input.e != input.w) {
      	  tab_times ++;
          //cprintf("%d %d", tab_times, match_length);
          if (tab_times == 1) {
              match_length = 0;
          } 
          else {
              while (match_length != 0) {
                  if(input.e != input.w){
                      input.e--;
                      consputc(BACKSPACE);
                      if(len > 0) len--;
	                  concatInput();
                  }
                  match_length--;
              }
          }
          i = input.e - 1;
          while (input.buf[i % INPUT_BUF] != ' ' && i != input.w) i--;
          length = 0;
          while (i != input.e) commondPrefix[length++] = input.buf[i++ % INPUT_BUF];
          commondPrefix[length++] = '\0';
          //cprintf("%s %d", commondPrefix, tab_times);
          char* match_ans;
          match_ans = match_commond(commondPrefix);
          if (match_ans != NULL) {
              i = length - 1;
              while (match_ans[i] != '\0') {
                  input.buf[input.e++ % INPUT_BUF] = match_ans[i];
                  str[len++] = match_ans[i];
                  consputc(match_ans[i++]);
                  match_length++;
                  //cprintf("%d", match_length);
              }
          } 
      }
      break;
    case C('U'):  // Kill line.
      his.flag = 0;//history flag
      bufferPos = 0;
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
        if(len > 0) len--;
        concatInput();
      }
      break;
    //case 0xE2: Test for up key
	case 0xE2:
        if (his.recordNum == 0)
           continue;
        if(his.flag == 0)
        {
            his.pos = his.current - 1;
            if (his.pos < 0) 
                his.pos = his.recordNum-1;
            his.flag = 1;
        }
        else
        {
            if (his.pos  == his.current )
               continue;
            if (his.recordNum < MAX_HISTORY && his.pos == 0)
               continue;
            his.pos--;
            if (his.pos < 0)
               his.pos = his.recordNum-1;

        }
	bufferPos = 0;
	    if(his.record == 1)
	    {
            while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
                input.e--;
                consputc(BACKSPACE);
            }

            len = 0;
		    int i;

		    for(i = 0; i < (sizeof(his.cmds[his.pos]) / sizeof(char)); i++)
		    {
                if(his.cmds[his.pos][i] == '\0') break;
			    input.buf[input.e++ % INPUT_BUF] = his.cmds[his.pos][i];
			    consputc(his.cmds[his.pos][i]);
                str[len++] = his.cmds[his.pos][i];
		    }

	    }
	  break;
    //case 0xE3: Test for down key
    case 0xE3:	
           if (his.recordNum == 0)
              continue;
            if(his.flag == 0)
            {
               continue;
            }
            else
            {
                int now;
                now = his.current - 1;
                if (now < 0)
                   now = his.recordNum-1; 
                if (his.pos  == now)
                   continue;
                if (his.recordNum < MAX_HISTORY && his.pos == his.recordNum - 1)
                   continue;
                his.pos++;
                if (his.pos >= his.recordNum)
                   his.pos = 0; 
            }
	    bufferPos = 0;
			if(his.record == 1)
			{
                while(input.e != input.w &&
                input.buf[(input.e - 1) % INPUT_BUF] != '\n'){
                    input.e--;
                    consputc(BACKSPACE);
                }

                len = 0;
				int i;

				for(i = 0; i < (sizeof(his.cmds[his.pos]) / sizeof(char)); i++)
				{
                    if(his.cmds[his.pos][i] == '\0') break;
					input.buf[input.e++ % INPUT_BUF] = his.cmds[his.pos][i];
					consputc(his.cmds[his.pos][i]);
                    str[len++] = his.cmds[his.pos][i];
				}
			}
	  break; 
  //Test
	case 0xE4:
	    if(input.e != input.w){
        input.e--;
        //consputc(BACKSPACE);
        consputc(0xe4);
        if(len > 0) len--;
      }
      break;
    /*
    case 0xE5:
      if (bufferPos > 0)
      {
          input.buf[input.e++ % INPUT_BUF] = buffer[bufferPos-1];
          str[len++] = buffer[bufferPos-1];//Preparation for building a history record
          consputc(buffer[bufferPos-1]);
          bufferPos --;
          concatInput();
      }
 
       
	
      break;
    */
  //
    /*
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    */
    case 0xE5:
	if (bufferPos > 0)
	{
		bufferPos --;
	    	input.buf[input.e++ % INPUT_BUF] = buffer[bufferPos];
		str[len++] = buffer[bufferPos];//Preparation for building a history record
		consputc(buffer[bufferPos]);
		concatInput();
	}
	break;

    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        if(c == '\n'){
            int i;
      		for(i = bufferPos - 1; i >= 0; i--)
		    {
			    input.buf[input.e++ % INPUT_BUF] = buffer[i];
			    str[len++] = buffer[i];
		    }
		    bufferPos = 0;
		}    
        input.buf[input.e++ % INPUT_BUF] = c;
        str[len++] = c;//Preparation for building a history record
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF){
          his.flag = 0;
          input.w = input.e;
          str[len - 1] = '\0';//Add a '\0' in the end
          recordHistory(str);
          len = 0;//reset len for the next recording preparation
          wakeup(&input.r);
        }
        else{
            concatInput();
        }
      }
	
      break;
    }
  }
  release(&input.lock);
  }
}