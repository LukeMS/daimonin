/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	The author can be reached via e-mail to info@daimonin.net
*/
#include <global.h>

/* attach command buffer to command list of this ns socket */
void command_buffer_enqueue(NewSocket *ns, command_struct *cmdptr)
{

	LOG(-1,"ENQUEUE CMD: cmdptr:%x\n", cmdptr);
	if(ns->cmd_start)
	{
		ns->cmd_end->next = cmdptr;
		cmdptr->last = ns->cmd_end;
		cmdptr->next = NULL;
		ns->cmd_end = cmdptr;
	}
	else
	{
		ns->cmd_end = ns->cmd_start = cmdptr;
		cmdptr->next = cmdptr->last = NULL;
	}
}

/* release a single command buffer to the mempool */
void command_buffer_clear(NewSocket *ns)
{
	command_struct *cmdtmp;

	cmdtmp = ns->cmd_start;
	ns->cmd_start = ns->cmd_start->next;
	if(!ns->cmd_start)
		ns->cmd_end = NULL;
	return_poolchunk(cmdtmp, cmdtmp->pool);
}

/* clear a command buffer queue and release all command buffers to the mempool */
void command_buffer_queue_clear(NewSocket *ns)
{
	while (ns->cmd_start)
		command_buffer_clear(ns);
}

/* allocate the initial command buffers from the mempool */
void initialize_command_buffer16(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(16);
}
void initialize_command_buffer32(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(32);
}
void initialize_command_buffer64(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(64);
}
void initialize_command_buffer128(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(128);
}
void initialize_command_buffer256(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(256);
}
void initialize_command_buffer1024(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(1024);
}
void initialize_command_buffer4096(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(4096);
}
