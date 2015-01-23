/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __SOCKPROTO_H
#define __SOCKPROTO_H

/* account.c */
void            account_clear(Account *ac);
account_status  account_save(Account *ac, const char *name);
account_status  account_create(Account *ac, const char *name, char *pass);
account_status  account_load(Account *ac, char *name, char *pass);
void            account_create_msg(NewSocket *ns, int msg);
void            account_send_client(NewSocket *ns, int stats);
account_status  account_delete_player(NewSocket *ns, shstr_t *name);
Account        *account_get_from_object(object_t *op);
int             account_update(Account *ac, object_t *op);
Account        *find_account(char *acname);
Account        *find_account_hash(const char *acname);
/* commands.c */
CommArray_s    *find_command(char *cmd, player_t *pl);
CommArray_s    *find_command_element(char *cmd, CommArray_s *commarray, int commsize);
void            process_command_queue(NewSocket *ns, player_t *pl);
void            cs_cmd_ping(char *buf, int len, NewSocket *ns);
void            cs_cmd_generic(char *buf, int len, NewSocket *ns);
void            cs_cmd_setup(char *buf, int len, NewSocket *ns);
void            cs_cmd_addme(char *buf, int len, NewSocket *ns);
void            cs_cmd_reply(char *buf, int len, NewSocket *ns);
void            cs_cmd_file(char *buf, int len, NewSocket *ns);
void            cs_cmd_newchar(char *buf, int len, NewSocket *ns);
void            cs_cmd_delchar(char *buf, int len, NewSocket *ns);
void            cs_cmd_moveobj(char *buf, int len, NewSocket *ns);
void            cs_cmd_face(char *params, int len, NewSocket *ns);
void            cs_cmd_move(char *params, int len, NewSocket *ns);
void            cs_cmd_examine(char *buf, int len, NewSocket *ns);
void            cs_cmd_apply(char *buf, int len, NewSocket *ns);
void            cs_cmd_lock(char *data, int len, NewSocket *ns);
void            cs_cmd_mark(char *data, int len, NewSocket *ns);
void            cs_cmd_talk(char *data, int len, NewSocket *ns);
void            cs_cmd_fire(char *params, int len, NewSocket *ns);
void            cs_cmd_checkname(char *buf, int len, NewSocket *ns);
void            cs_cmd_login(char *buf, int len, NewSocket *ns);
/* image.c */
#ifdef SERVER_SEND_FACES
int             is_valid_faceset(int fsn);
void            free_socket_images(void);
void            read_client_images(void);
int             esrv_send_face(NewSocket *ns, short face_num, int nocache);
#endif
/* info.c */
void            ndi(const int flags, const int pri, const object_t *const op, const char *const format, ...) DAI_GNUC_PRINTF(4, 5);
void            ndi_map(const int flags, msp_t *msp, const int dist, const object_t *const except1, const object_t *const except2, const char *const format, ...) DAI_GNUC_PRINTF(6, 7);
/* init.c */
void            InitConnection(NewSocket *ns, char *str_ip);
void            init_ericserver(void);
void            free_all_newserver(void);
NewSocket		*socket_get_available(void);
void            close_newsocket(NewSocket *ns);
void            free_newsocket(NewSocket *ns);
/* item.c */
void            esrv_send_below(player_t *pl);
void            esrv_send_inventory(player_t *pl, object_t *op);
void            esrv_open_container(player_t *pl, object_t *op);
void            esrv_close_container(player_t *pl);
void            esrv_send_item(object_t *op);
void            esrv_update_item(uint16 flags, object_t *op);
void            esrv_del_item(object_t *op);
void            esrv_send_or_del_item(object_t *op);
object_t         *esrv_get_ob_from_count(object_t *who, tag_t count);
/* loop.c */
void            remove_ns_dead_player(player_t *pl);
void            doeric_server(int update, struct timeval *timeout);
/* lowlevel.c */
void			write_socket_buffer(NewSocket *ns);
int				read_socket_buffer(NewSocket *ns);
/* metaserver.c */
void            metaserver_init(void);
void            metaserver_update(void);
/* read.c */
void            command_buffer_queue_clear(NewSocket *ns);
void			command_buffer_clear(NewSocket *ns);
void			command_buffer_enqueue(NewSocket *ns, command_struct *cmdptr);
void            initialize_command_buffer16(command_struct *cmdbuf);
void            initialize_command_buffer32(command_struct *cmdbuf);
void            initialize_command_buffer64(command_struct *cmdbuf);
void            initialize_command_buffer128(command_struct *cmdbuf);
void            initialize_command_buffer256(command_struct *cmdbuf);
void            initialize_command_buffer1024(command_struct *cmdbuf);
void            initialize_command_buffer4096(command_struct *cmdbuf);
/* request.c */
void            send_target_command(player_t *pl);
void            send_spelllist_cmd(object_t *op, char *spellname, int mode);
void            send_skilllist_cmd(player_t *pl, int snr, int mode);
void            send_ready_skill(player_t *pl, shstr_t *name);
void            send_golem_control(object_t *golem, int mode);
void            esrv_update_stats(player_t *pl);
void            esrv_new_player(player_t *pl, uint32 weight);
/* startup.c */
void            init_srv_files(void);
/* sounds.c */
void            init_sounds(void);
void            free_sounds(void);
int             lookup_sound(int type_id, const char* soundname);
void            play_sound_player_only(player_t *pl, int soundnum, int soundtype, int x, int y);
void            play_sound_map(msp_t *msp, int sound_num, int sound_type);
/* write.c */
char			*socket_buffer_request(NewSocket *ns, int data_len);
sockbuf_struct	*socket_buffer_adjust(sockbuf_struct *sbuf, int len);
void			socket_buffer_request_finish(NewSocket *ns, int cmd, int len);
void			socket_buffer_request_reset(NewSocket *ns);
sockbuf_struct	*socket_buffer_get(int len);
sockbuf_struct	*compose_socklist_buffer(int cmd, char *data, int data_len, int flags);
void			socket_buffer_enqueue(NewSocket *ns, sockbuf_struct *sockbufptr);
void			socket_buffer_dequeue(NewSocket *ns);
void			socket_buffer_queue_clear(NewSocket *ns);
void			initialize_socket_buffer_small(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_medium(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_huge(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_dynamic(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_broadcast(sockbuf_struct *sockbuf);
void			free_socket_buffer_dynamic(sockbuf_struct* sb);
void			free_socket_buffer_broadcast(sockbuf_struct* sb);

#endif /* ifndef __SOCKPROTO_H */
