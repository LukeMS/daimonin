To use doxygen, simply change to this directory and type "doxygen".
Doxygen defaults to the configuration file "Doxyfile", so no arguments are
required.

The generated documentation is output to the following path:
relative (from here): ../../doxydoc/html/
absolute: daimonin/server/doxydoc/html/

The start document is named "index.html".


How to document?

The following example shows how to document

/** In a movement behaviour, add <var>dir</var> to the list of forbidden
 * directions, but don't select a specific movement.
 * @note Lua:    ai:ForbidMoveDirection(dir)
 * @param        L        lua_State for Lua environment
 * @param        Lua:dir  forbidden movement direction
 * @return       number of lua return values
 * @return       Lua:void
 * @note Status: Tested
 **/
static int AI_ForbidMoveDirection(lua_State *L)
{
    // ...
}


Explanation
-----------
First Sentence: The first sentence (really the sentence, up to the first full
        stop (period, "."), is used as a brief description. The brief
        description is used for summaries and overviews. Therefor the first
        sentence should be as concise as possible.
@param is used to document the named function parameter. For functions
        with multiple parameters, use multiple @param lines.
        Synopsis: @param <parametername> <argumentdescription>
@return is used to document the return value of a function.
        Synopsis: @return <returnvaluedescription>
@note Status: Please use a form like this to document wether you've tested
        a function.

Lua Plugin Special
------------------
@note Lua: is used to document the Lua synopsis of a Lua plugin method.
@param Lua: is used to document a Lua parameter
@return Lua: is used to document the Lua return values
Doxygen warrns about this @param.
I haven't yet found a better way to document Lua meta parameters and return.
If someone finds out how to teach doxygen new commands, tell me.

Pitfalls
--------

The following does *NOT* work:
/*****************************************************************************/
/* comment lines */
/*****************************************************************************/
Why? Because this isn't one, this is many comments.

The following does *NOT* work:
/******************************************************************************
 * In a movement behaviour, add <var>dir</var> to the list of forbidden
 * directions, but don't select a specific movement.
 * @note Lua:    ai:ForbidMoveDirection(dir)
 * @param        L        lua_State for Lua environment
 * @param        Lua:dir  forbidden movement direction
 * @return       number of lua return values
 * @return       Lua:void
 * @note Status: Tested
 **/
Why? Because /** introduces the doxygen comment and the following asterisks
are included in the documentation.

The following DOES work:
/**
 * In a movement behaviour, add <var>dir</var> to the list of forbidden
 * directions, but don't select a specific movement.
 * @note Lua:    ai:ForbidMoveDirection(dir)
 * @param        L        lua_State for Lua environment
 * @param        Lua:dir  forbidden movement direction
 * @return       number of lua return values
 * @return       Lua:void
 * @note Status: Tested
 *****************************************************************************/

Perhaps we should use it? This is a point to discuss first.
