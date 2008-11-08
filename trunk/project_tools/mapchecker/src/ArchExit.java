/*
 * ArchExit.java
 *
 * Created on 10 May 2007, 10:21
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class ArchExit
{
    private String  name;
    private boolean walk_on;
    private boolean fly_on;
    
    /** Creates a new instance of ArchExit */
    public ArchExit(String n, boolean w, boolean f)
    {
        name = n;
        walk_on = w;
        fly_on = f;
    }
    
    public boolean getWalkOn()
    {
        return walk_on;
    }
    
    public boolean getFlyOn()
    {
        return fly_on;
    }
}
