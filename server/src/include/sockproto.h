/* account.c */
void            account_clear(Account *ac);
account_status  account_save(Account *ac, const char *name);
account_status  account_create(Account *ac, const char *name, char *pass);
account_status  account_load(Account *ac, char *name, char *pass);
void            account_create_msg(NewSocket *ns, int msg);
void            account_send_client(NewSocket *ns, int stats);
account_status  account_delete_player(Account *ac, char *name);
int             account_name_valid(char *cp);
/* commands.c */
CommArray_s     *find_command_element(char *cmd, CommArray_s *commarray, int commsize);
void            process_command_queue(NewSocket *ns, player *pl);
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
void            new_draw_info(const int flags, const int pri, const object *const pl, const char *const buf);
void            new_draw_info_format(const int flags, const int pri, const object *const pl, const char *const format, ...);
void            new_info_map(const int color, const mapstruct *const map, const int x, const int y, const
                             int dist, const char *const str);
void            new_info_map_except(const int color, const mapstruct *const map, const int x, const int y,
                                    const int dist, const object *const op1, const object *const op,
                                    const char *const str);
void            new_info_map_format(const int color, const mapstruct *const map, const int x, const int y,
                                    const int dist, const char *const format, ...);
void            new_info_map_except_format(const int color, const mapstruct *const map, const int x, const int y,
                                            const int dist, const object *const op1, const object *const op,
                                            const char *const format, ...);
/* init.c */
void            InitConnection(NewSocket *ns, char *str_ip);
void            init_ericserver(void);
void            free_all_newserver(void);
NewSocket		*socket_get_available(void);
void            close_newsocket(NewSocket *ns);
void            free_newsocket(NewSocket *ns);
/* item.c */
unsigned int    query_flags(object *op);
void            esrv_draw_look(object *pl);
int             esrv_draw_DM_inv(object *pl, object *op);
void            esrv_close_container(object *op);
void            esrv_send_inventory(object *pl, object *op);
void            esrv_update_item(int flags, object *pl, object *op);
void            esrv_send_item(object *pl, object *op);
void            esrv_del_item(player *pl, int tag, object *cont);
object         *esrv_get_ob_from_count(object *pl, tag_t count);
void            esrv_move_object(object *pl, tag_t to, tag_t tag, long nrof);
/* loop.c */
void            remove_ns_dead_player(player *pl);
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
void            esrv_update_skills(player *pl);
void            esrv_update_stats(player *pl);
void            esrv_new_player(player *pl, uint32 weight);
/* startup.c */
void            init_srv_files(void);
/* sounds.c */
void            play_sound_player_only(player *pl, int soundnum, int soundtype, int x, int y);
void            play_sound_map(mapstruct *map, int x, int y, int sound_num, int sound_type);
/* write.c */
char			*socket_buffer_request(NewSocket *ns, int data_len);
sockbuf_struct	*socket_buffer_adjust(sockbuf_struct *sbuf, int len);
void			socket_buffer_request_finish(NewSocket *ns, int cmd, int len);
void			socket_buffer_request_reset(NewSocket *ns);
sockbuf_struct	*socket_buffer_get(int len);
sockbuf_struct	*compose_socklist_buffer(int cmd, sockbuf_struct *out_buf, char *cmd_buf, int len, int flags);
void			socket_buffer_enqueue(NewSocket *ns, sockbuf_struct *sockbufptr);
void			socket_buffer_dequeue(NewSocket *ns);
void			socket_buffer_queue_clear(NewSocket *ns);
void			initialize_socket_buffer_small(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_medium(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_huge(sockbuf_struct *sockbuf);
void			initialize_socket_buffer_dynamic(sockbuf_struct *sockbuf);
void			free_socket_buffer_dynamic(sockbuf_struct* sb);
