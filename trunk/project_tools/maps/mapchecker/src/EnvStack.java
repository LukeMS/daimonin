/*
 * EnvStack.java
 *
 * Created on 08 May 2007, 18:46
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

import java.util.*;

/**
 *
 * @author Jeff
 */
public class EnvStack
{
    
    private LinkedList<DaiEnv> list = new LinkedList<DaiEnv>();

    public void push(DaiEnv item)
    {
            list.addFirst(item);
    }

    public DaiEnv pop()
    {
            return list.poll();
    }

    public DaiEnv peek()
    {
            return list.peek();
    }

    public boolean hasItems()
    {
            return !list.isEmpty();
    }

    public int size()
    {
            return list.size();
    }
    
    /**
     * Get compound description.
     * If name is set, use that, otherwise arch name.
     */
    public String getDesc()
    {
        String  desc = "map";
        String  locName;
        DaiEnv  entry;

        if (hasItems())
        {
            entry = list.peek();
            desc = "map (" + entry.x + ", " + entry.y + ")";
        }
        for (int i = list.size() - 1; i >= 0; i--)
        {
            entry = list.get(i);
            locName = entry.arch;
            if (entry.name != null)
                locName += " ('" + entry.name + "')";
            desc = locName + " in " + desc;
        }
        return desc;
    }
}
