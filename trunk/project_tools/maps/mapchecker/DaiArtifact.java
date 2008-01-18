/*
 * DaiArtifact.java
 *
 * Created on 28 December 2007, 14:52
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiArtifact
{
    private String artifact;
    private String name;
    private String title;
    private String def_arch;
    private String path;
    private int type;
    private int layer;
    
    /** Creates a new instance of DaiArtifact */
    public DaiArtifact(String a, String n, String t, String d, String p, int ty, int l)
    {
        artifact = a;
        name     = n;
        title    = t;
        def_arch = d;
        path     = p;
        type     = ty;
        layer    = l;
    }

    public String getName()
    {
        return name;
    }
    
    public String getTitle()
    {
        return title;
    }
    
    public String getDefArch()
    {
        return def_arch;
    }
    
    public String getPath()
    {
        return path;
    }

    public int getType()
    {
        return type;
    }
    
    public int getLayer()
    {
        return layer;
    }    
}
