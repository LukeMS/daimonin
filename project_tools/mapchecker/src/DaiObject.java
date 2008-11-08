/*
 * DaiObject.java
 *
 * Created on 10 October 2007, 16:49
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiObject
{
    private String object;
    private String name;
    private String title;
    private String other_arch;
    private int type;
    private int layer;
    private int envLevel;
    private boolean system;
    
    /**
     * Creates a new instance of DaiObject
     */
    public DaiObject(String o, String n, String tt, String oa, int t, int l, int e, boolean s)
    {
        object      = o;
        name        = n;
        title       = tt;
        other_arch  = oa;
        type        = t;
        layer       = l;
        envLevel    = e;
        system      = s;
    }
    
    public String getObject()
    {
        return object;
    }
    
    public String getName()
    {
        return name;
    }

    public String getTitle()
    {
        return title;
    }
    
    public String getOtherArch()
    {
        return other_arch;
    }
    
    public int getType()
    {
        return type;
    }
    
    public int getLayer()
    {
        return layer;
    }
    
    public int getEnvLevel()
    {
        return envLevel;
    }
    
    public boolean isSysObject()
    {
        return system;
    }
}
