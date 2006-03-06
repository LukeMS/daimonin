#!/usr/bin/python

import htmlentitydefs, os, os.path, re, sys, urllib

length = len(sys.argv)
if length != 2 and length != 3:
	print 'Usage: create_lua_doc.py <source_directory> [destination]'
	print 'Example: ./create_lua_doc.py ../../server/src/plugin_lua /tmp'
	sys.exit(0)

if length is 3:
	dest = sys.argv[2]
else:
	dest = os.getcwd()

if not os.path.isdir(dest):
	print dest + " is not a directory"
	sys.exit(1)

index_items_per_line = 6

amp_re_obj = re.compile('&')
attributes_re_obj = re.compile('\{\s*"(.*?)",\s*FIELDTYPE_(.*?),\s*offsetof\(.*?\),\s*(.*?)\s*[\},]')
block_re_obj = re.compile('FUNCTIONSTART.*?FUNCTIONEND', re.S)
field_start_re_obj = re.compile('(Lua|Info|Status|TODO|Warning|Remark)\s*:\s*(.*)')
colon_re_obj = re.compile('\s*:\s*')
colon_find_re_obj = re.compile('\s*.+?:')
colon_prefix_re_obj = re.compile('^\s*:\s*')
comment_re_obj = re.compile('/\*+/[\n\r]+(.*)[\n\r]+/\*+/', re.S)
comment_prefix_re_obj = re.compile('/\*\s*')
comment_suffix_re_obj = re.compile('\s*\*/[\n\r]*')
constants_re_obj = re.compile('"(.+?)"')
dot_re_obj = re.compile('\.')
flags_re_obj = re.compile('"(.+?)"')
func_re_obj = re.compile('/\*.*?[\n\r]+\}', re.S)
func_body_re_obj = re.compile('\s*static\s+int\s+.+?\(lua_State\s*\*\s*L\).*?\{(.+)\}', re.S)
game_constants_block_re_obj = re.compile('\s*static\s+struct\s+constant_decl\s+Game_constants\[\]\s+=[\n\r]+\{(.*?)[\n\r]+\};', re.S)
gt_re_obj = re.compile('>')
lt_re_obj = re.compile('<')
event_attributes_block_re_obj = re.compile('\s*struct\s+attribute_decl\s+Event_attributes\[\]\s+=[\n\r]+(.*?)[\n\r]+\};', re.S)
map_attributes_block_re_obj = re.compile('\s*struct\s+attribute_decl\s+Map_attributes\[\]\s+=[\n\r]+(.*?)[\n\r]+\};', re.S)
map_flags_block_re_obj = re.compile('\s*static\s+const\s+char\s*\*\s*Map_flags\[\]\s*=[\n\r]+(.*?)[\n\r]+\};', re.S)
name_prefix_re_obj = re.compile('([A-Za-z_]*):.+')
name_re_obj = re.compile('[A-Za-z_]*[\.:](.*)\(')
newline_re_obj = re.compile('\n')
object_attributes_block_re_obj = re.compile('\s*struct\s+attribute_decl\s+GameObject_attributes\[\]\s+=[\n\r]+(.*?)[\n\r]+\};', re.S)
object_flags_block_re_obj = re.compile('\s*static\s+const\s+char\s*\*\s*GameObject_flags\[.*?\]\s*=[\n\r]+(.*?)[\n\r]+\};', re.S)
parameter_names_re_obj = re.compile('.*\((.*)\)')
parameter_types_re_obj = re.compile('get_lua_args\(L,\s*"([GMAOdfiIs\|?]+)"')
quot_re_obj = re.compile('"')
return_boolean_re_obj = re.compile('.*lua_pushboolean\(.*?[\n\r]+\s*return\s*1;', re.S)
return_number_re_obj = re.compile('.*lua_pushnumber\(.*?[\n\r]+\s*return\s*1;', re.S)
return_map_re_obj = re.compile('.*return\s*push_object\(L,\s*&Map,', re.S)
return_nothing_re_obj = re.compile('.*return\s*0', re.S)
return_string_re_obj = re.compile('.*lua_pushstring\(.*?[\n\r]+\s*return\s*1;', re.S)
return_object_re_obj = re.compile('.*return\s*push_object\(L,\s*&GameObject,', re.S)
return_ai_re_obj = re.compile('.*return\s*push_object\(L,\s*&AI,', re.S)
#return_array_re_obj = re.compile('.*return\s*push_object\(L,\s*&GameObject,', re.S)
return_array_re_obj = re.compile('.*lua_newtable\(L\).*lua_rawseti\(L.*.*return\s+1', re.S)
return_table_re_obj = re.compile('.*lua_newtable\(L\).*lua_rawset\(L.*.*return\s+1', re.S)

def end(f):
	f.write('</body></html>')
	f.close()

def entities(string):
	return newline_re_obj.sub('<br />', gt_re_obj.sub('&gt;', lt_re_obj.sub('&lt;', quot_re_obj.sub('&quot;', amp_re_obj.sub('&amp;', string)))))

def listCFiles(directory):
	if not os.path.isdir(directory):
		print directory + " is not a directory"
		sys.exit(1)
	lst = os.listdir(directory)
	r = []
	for i in lst:
		p = os.path.join(directory, i)
		if os.path.isdir(p):
			r += listCFiles(p)
		elif p.endswith('.c') and not os.path.basename(p).startswith('.'):
			r.append(p)
	return r

def start(filename, title):
	f = file(os.path.join(dest, filename + '.html'), 'w')
	f.write('''<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/dtd/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"><head><title>Daimonin Lua Core reference - ''' + title + '</title></head><body><h1>Daimonin Lua Core reference - ' + title + '</h1>')
	return f

doc = {}
map_attributes = {}
map_flags = []
object_attributes = {}
object_flags = []
for filename in listCFiles(sys.argv[1]):
	f = file(filename, 'r')
	code = f.read()
	f.close()
	blocks = block_re_obj.findall(code)
	for block in blocks:
		functions = func_re_obj.findall(block)
		for function in functions:
			fields = {}
			comments = comment_re_obj.findall(function)
			for comment in comments:
				comment = comment_suffix_re_obj.split(comment_prefix_re_obj.sub('', comment))
				key = None
				for line in comment:
					match = field_start_re_obj.findall(line)
					if match:
						match = match[0]
						if match[0] != 'Name':
							key = match[0]
							fields[key] = match[1]
					elif key != None and line:
						fields[key] += "\n" + colon_prefix_re_obj.sub('', line)


			if 'Lua' in fields:
				prefix = name_prefix_re_obj.findall(fields['Lua'])
				if prefix:
					prefix = prefix[0]
					body = func_body_re_obj.findall(function)
					if body:
						body = body[0]
						if prefix == 'map' or prefix == 'object' or prefix == 'ai':
							fields['Lua'] = dot_re_obj.sub(':', fields['Lua'], 1)
						c = 0
						optional = 0
						parameters = ([], [])
						names = parameter_names_re_obj.findall(fields['Lua'])
						names = names[0].split(',')
						length = len(names)
						nil = 0
						types = parameter_types_re_obj.findall(body)
						if len(types) > 0:
							types = types[0]
						else:
							parameters[0].append((names[0], 'unknown'))
						for i in range(len(types)):
							tp = ''
							if types[i] == 's':
								tp = 'string'
							elif types[i] == 'i':
								tp = 'integer'
							elif types[i] == 'I':
								tp = 'big integer (64 bit)'
							elif types[i] == 'f':
								tp = 'float'
							elif types[i] == 'd':
								tp = 'double'
							elif types[i] == 'O':
								if i != 0 or prefix != 'object':
									tp = 'object'
							elif types[i] == 'G':
								if i != 0 or prefix != 'game':
									tp = 'game'
							elif types[i] == 'M':
								if i != 0 or prefix != 'map':
									tp = 'map'
							elif types[i] == 'A':
								if i != 0 or prefix != 'ai':
									tp = 'ai'
							elif types[i] == '|':
								optional = 1
							elif types[i] == '?':
								nil = 1
							if tp:
								if c < length:
									key = names[c]
								else:
									key = tp
								if nil:
									nil = 0
									tp += ' or nil'
								parameters[optional].append((key, tp))
								c += 1

						fields['parameters'] = parameters
						return_types = []
						match = return_boolean_re_obj.match(body)
						if match != None:
							return_types.append('boolean')
						match = return_number_re_obj.match(body)
						if match != None:
							return_types.append('number')
						match = return_string_re_obj.match(body)
						if match != None:
							return_types.append('string')
						match = return_object_re_obj.match(body)
						if match != None:
							return_types.append('object')
						match = return_map_re_obj.match(body)
						if match != None:
							return_types.append('map')
						match = return_nothing_re_obj.match(body)
						if match != None:
							return_types.append('nil')
						match = return_ai_re_obj.match(body)
						if match != None:
							return_types.append('ai')

						# Try to figure out arrays
						# (lua_newtable(), push_object(), lua_rawseti()) combo
						match = return_array_re_obj.match(body)
						if match != None:
							return_types.append('array of something')
						match = return_table_re_obj.match(body)
						if match != None:
							return_types = ['table of something'] # overrides other types

						try:
							last = return_types[-1]
						except:
							print body
						return_types = return_types[:-1]
						if return_types:
							fields['return'] = ', '.join(return_types) + ' or ' + last
						else:
							fields['return'] = last

					if not prefix in doc:
						doc[prefix] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}

					key = name_re_obj.findall(fields['Lua'])
					key = key[0]
					doc[prefix]['functions'][key] = fields

	if not 'game' in doc:
		doc['game'] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}
	block = game_constants_block_re_obj.findall(code)
	if block:
		constants = constants_re_obj.findall(block[0])
		if constants:
			for constant in constants:
				doc['game']['constants'].append(constant)

	if not 'event' in doc:
		doc['event'] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}
	block = event_attributes_block_re_obj.findall(code)
	if block:
		attributes = attributes_re_obj.findall(code)
		if attributes:
			for attribute in attributes:
				special = ''
				tp = ''
				if attribute[1] == 'CSTR' or attribute[1] == 'SHSTR':							tp = 'string'
				elif attribute[1] == 'SINT8' or attribute[1] == 'SINT16' or attribute[1] == 'SINT32' or attribute[1] == 'UINT8' or attribute[1] == 'UINT16' or attribute[1] == 'UINT32':
					tp = 'integer'
				elif attribute[1] == 'FLOAT':
					tp = 'float'
				elif attribute[1] == 'OBJECTREF' or attribute[1] == 'OBJECT':
					tp = 'object'
				if attribute[2] == 'FIELDFLAG_READONLY':
					special = 'readonly'
				elif attribute[2] == 'FIELDFLAG_PLAYER_READONLY':
					special = 'readonly if object is a player'
				elif attribute[2] == 'FIELDFLAG_PLAYER_FIX':
					special = 'fix the player or mob after change'
				if tp:
					doc['event']['attributes'][attribute[0]] = (tp, special)

	if not 'map' in doc:
		doc['map'] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}
	block = map_attributes_block_re_obj.findall(code)
	if block:
		attributes = attributes_re_obj.findall(code)
		if attributes:
			for attribute in attributes:
				special = ''
				tp = ''
				if attribute[1] == 'CSTR' or attribute[1] == 'SHSTR':
					tp = 'string'
				elif attribute[1] == 'SINT8' or attribute[1] == 'SINT16' or attribute[1] == 'SINT32' or attribute[1] == 'UINT8' or attribute[1] == 'UINT16' or attribute[1] == 'UINT32':
					tp = 'integer'
				elif attribute[1] == 'FLOAT':
					tp = 'float'
				elif attribute[1] == 'OBJECTREF' or attribute[1] == 'OBJECT':
					tp = 'object'
				if attribute[2] == 'FIELDFLAG_READONLY':
					special = 'readonly'
				elif attribute[2] == 'FIELDFLAG_PLAYER_READONLY':
					special = 'readonly if object is a player'
				elif attribute[2] == 'FIELDFLAG_PLAYER_FIX':
					special = 'fix the player or mob after change'
				if tp:
					doc['map']['attributes'][attribute[0]] = (tp, special)

	block = map_flags_block_re_obj.findall(code)
	if block:
		flags = flags_re_obj.findall(block[0])
		if flags:
			for flag in flags:
				if flag.startswith('?'):
					doc['map']['flags'][flag.strip('?')] = {'readonly': 1}
				else:
					doc['map']['flags'][flag] = {}

	if not 'object' in doc:
		doc['object'] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}
	block = object_attributes_block_re_obj.findall(code)
	if block:
		attributes = attributes_re_obj.findall(code)
		if attributes:
			for attribute in attributes:
				special = ''
				tp = ''
				if attribute[1] == 'CSTR' or attribute[1] == 'SHSTR':
					tp = 'string'
				elif attribute[1] == 'SINT8' or attribute[1] == 'SINT16' or attribute[1] == 'SINT32' or attribute[1] == 'UINT8' or attribute[1] == 'UINT16' or attribute[1] == 'UINT32' or attribute[1] == 'SINT64':
					tp = 'integer'
				elif attribute[1] == 'FLOAT':
					tp = 'float'
				elif attribute[1] == 'OBJECTREF' or attribute[1] == 'OBJECT':
					tp = 'object'
				if attribute[2] == 'FIELDFLAG_READONLY':
					special = 'readonly'
				elif attribute[2] == 'FIELDFLAG_PLAYER_READONLY':
					special = 'readonly if object is a player'
				elif attribute[2] == 'FIELDFLAG_PLAYER_FIX':
					special = 'fix the player or mob after change'
				if tp:
					doc['object']['attributes'][attribute[0]] = (tp, special)

	block = object_flags_block_re_obj.findall(code)
	if block:
		flags = flags_re_obj.findall(block[0])
		if flags:
			for flag in flags:
				if flag.startswith('?'):
					doc['object']['flags'][flag.strip('?')] = {'readonly': 1}
				else:
					doc['object']['flags'][flag] = {}

	if not 'ai' in doc:
		doc['ai'] = {'attributes': {}, 'constants': [], 'flags': {}, 'functions': {}}

index = start('index', 'Index')
doc_keys = doc.keys()
doc_keys.sort()
first = 1
for key in doc_keys:
	if doc[key]['attributes'] or doc[key]['constants'] or doc[key]['flags'] or doc[key]['functions']:
		quoted = urllib.quote(key)
		if not first:
			index.write('<hr />')
		else:
			first = 0
		index.write('<h2>Class: <code><a href="' + quoted + '.html">' + entities(key) + '</a></code></h2>')
		f = start(key, key)
		if doc[key]['constants']:
			index.write('<h3><a href="' + quoted + '.html#constants">Constants</a></h3><p><code>')
			f.write('<hr /><h2><a id="constants">Constants</a></h2><p><code>')
			constants = doc[key]['constants']
			constants.sort()
			count = 0
			for constant in constants:
				quoted2 = urllib.quote(constant)
				index.write('<a href="' + quoted + '.html#' + quoted2 + '">' + entities(constant) + '</a>')
				count = count + 1
				if count == index_items_per_line:
					count = 0
					index.write('<br />')
				else:
					index.write(' | ')
				f.write('<b><a id="' + quoted2 + '">' + entities(key + '.' + constant) + '</a></b><br />')
			f.write('</code></p><p><a href="index.html">Back to the index</a></p>')
			index.write('</code></p>')
		if doc[key]['attributes']:
			index.write('<h3><a href="' + quoted + '.html#attributes">Attributes</a></h3><p><code>')
			f.write('<hr /><h2><a id="attributes">Attributes</a></h2><p><code>')
			keys = doc[key]['attributes'].keys()
			keys.sort()
			count = 0
			for key2 in keys:
				attribute = doc[key]['attributes'][key2]
				quoted2 = urllib.quote(key2)
				index.write('<a href="' + quoted + '.html#' + quoted2 + '">' + entities(key2) + '</a>')
				count = count + 1
				if count == index_items_per_line:
					count = 0
					index.write('<br />')
				else:
					index.write(' | ')
				f.write(entities(attribute[0]) + ' <b><a id="' + quoted2 + '">' + entities(key + '.' + key2) + '</a></b>')
				if attribute[1]:
					f.write(' (' + entities(attribute[1]) + ')')
				f.write('<br />')
			f.write('</code></p><p><a href="index.html">Back to the index</a></p>')
			index.write('</code></p>')
		if doc[key]['flags']:
			index.write('<h3><a href="' + quoted + '.html#flags">Flags</a></h3><p><code>')
			f.write('<hr /><h2><a id="flags">Flags</a></h2><p><code>')
			keys = doc[key]['flags'].keys()
			keys.sort()
			count = 0
			for key2 in keys:
				flag = doc[key]['flags'][key2]
				quoted2 = urllib.quote(key2)
				index.write('<a href="' + quoted + '.html#' + quoted2 + '">' + entities(key2) + '</a>')
				count = count + 1
				if count == index_items_per_line:
					count = 0
					index.write('<br />')
				else:
					index.write(' | ')
				f.write('<b><a id="' + quoted2 + '">' + entities(key + '.' + key2) + '</a></b>')
				if "readonly" in flag:
					f.write(' (' + entities(attribute[1]) + ')')
				f.write('<br />')
			f.write('</code></p><p><a href="index.html">Back to the index</a></p>')
			index.write('</code></p>')
		if doc[key]['functions']:
			first2 = 1
			index.write('<h3><a href="' + quoted + '.html#functions">Functions</a></h3><p><code>')
			f.write('<hr /><h2><a id="functions">Functions</a></h2>')
			keys = doc[key]['functions'].keys()
			keys.sort()
			count = 0
			for key2 in keys:
				fields = doc[key]['functions'][key2]
				if 'Lua' in fields and 'Status' in fields:
					quoted2 = urllib.quote(key2)
					index.write('<a href="' + quoted + '.html#' + quoted2 + '">' + entities(key2) + '</a>')
					count = count + 1
					if count == index_items_per_line:
						count = 0
						index.write('<br />')
					else:
						index.write(' | ')
					if not first2:
						f.write('<hr />')
					else:
						first2 = 0
					f.write('<h3><code><a id="' + quoted2 + '">' + entities(fields['Lua']) + '</a></code></h3><p>');
					if 'parameters' in fields and (fields['parameters'][0] or fields['parameters'][1]):
						f.write('Parameter types:<br />')
						if fields['parameters'][0]:
							for parameter in fields['parameters'][0]:
								f.write('<i>' + entities(parameter[0]) + ':</i> ' + entities(parameter[1]) + ' (required)<br />')
						if fields['parameters'][1]:
							for parameter in fields['parameters'][1]:
								f.write('<i>' + entities(parameter[0]) + ':</i> ' + entities(parameter[1]) + ' (optional)<br />')
						f.write('</p><p>')
					if 'return' in fields and fields['return']:
						f.write('Return type: ' + entities(fields['return']) + '</p><p>')
					if 'Info' in fields and fields['Info']:
						f.write(entities(fields['Info']) + '</p><p>')
					f.write('Status: <b>' + entities(fields['Status']) + '</b></p><p>')
					if 'Warning' in fields and fields['Warning']:
						f.write('<b>Warning:</b> ' + entities(fields['Warning']) + '</p><p>')
					if 'Remark' in fields and fields['Remark']:
						f.write('<b>Remark:</b> ' + entities(fields['Remark']) + '</p><p>')
					if 'TODO' in fields and fields['TODO']:
						f.write('<b>TODO:</b> ' + entities(fields['TODO']) + '</p><p>')
					f.write('<a href="index.html">Back to the index</a></p>')
			index.write('</code></p>')
		end(f)

end(index)
