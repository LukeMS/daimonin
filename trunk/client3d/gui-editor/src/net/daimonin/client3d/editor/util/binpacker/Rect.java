/*
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * In addition, as a special exception, the copyright holders of client3d give
 * you permission to combine the client3d program with lgpl libraries of your
 * choice and/or with the fmod libraries.
 * You may copy and distribute such a system following the terms of the GNU GPL
 * for client3d and the licenses of the other code concerned.
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/licenses/licenses.html
 */
package net.daimonin.client3d.editor.util.binpacker;

/**
 * Rectangle, used by BinPacker.
 * 
 * @author Rumbuff.
 */
public class Rect implements Comparable {

	/**
	 * x position, if packed.
	 */
	int x;

	/**
	 * y position, if packed.
	 */
	int y;

	/**
	 * width.
	 */
	int w;

	/**
	 * height.
	 */
	int h;

	/**
	 * ID. Has to be unique within a packing run.
	 */
	String id;
	
	/**
	 * Internal constant, don't care about it.
	 */
	static final String ID_DEAULT = "invalid";	

	/**
	 * Internal data, don't care about it.
	 */
	private int[] children = new int[2];

	/**
	 * whether or not this rect got rotated during packing.
	 */
	boolean rotated;

	/**
	 * whether or not this rect was packed.
	 */
	boolean packed;

	/**
	 * Creates a Rect.
	 * @param id	Identifier of the rect. Has to be unique within all the rects in a BinPacker run.
	 * @param w		Width.
	 * @param h		Height.
	 */
	public Rect(final String id, final int w, final int h) {
		this(id, 0, 0, w, h, false, false);
	}
	
	/**
	 * Creates a Rect.
	 * @param id				Identifier of the rect. Has to be unique within all the rects in a BinPacker run.
	 * @param x					x coordinate, might be 0 if unknown.
	 * @param y					y coordinate, might be 0 if unknown.
	 * @param w					Width.
	 * @param h					Height.
	 * @param	rotated		if the rect is rotated.
	 * @param packed		if the rect was packed.
	 */
	Rect(final String id, final int x, final int y, final int w, final int h,
			final boolean rotated, final boolean packed) {
		this.id = id;
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		this.rotated = rotated;
		this.packed = packed;
		children[0] = -1;
	  children[1] = -1;
	}
	
	/**
	 * Copy constructor.
	 * @param rect  Rect to be copied.
	 */
	Rect(final Rect rect) {
		id = rect.id;
		x = rect.x;
		y = rect.y;
		w = rect.w;
		h = rect.h;
		children = rect.children;
		packed = rect.packed;
		rotated = rect.rotated;
	}

	/**
	 * Gets the area.
	 * @return	the area.
	 */
	final int getArea() {
		return w * h;
	}

	/**
	 * Rotates the rect (swaps w and h).
	 */
	final void rotate() {
		int tmp = h;
		h = w;
		w = tmp;
		rotated = !rotated;
	}
	
	/**
	 * Gets the child rects.
	 * @return	the child rects.
	 */
	final int[] getChildren() {
		return children;
	}
	
	/**
	 * Sets the child rects.
	 * @param c1	child rect 1.
	 * @param c2	child rect 2.
	 */
	final void setChildren(final int c1, final int c2) {
		children = new int[] {c1, c2};
	}

	/**
	 * Gets the main attributes as a string.
	 * @return  main attributes as a string.
	 */
	public final String toString() {
		return id + "/" + x + "/" + y + "/" + w + "/" + h + "/" + rotated + "/" + packed
		+ " (ID/x/y/w/h/rotated/packed)";
	}

	/**
	 * Compares this rect with another rect by area size.
	 * @param	rect	the rect to compare with.
	 * @return	@see java.lang.Comparable.compareTo() 
	 */
	public final int compareTo(final Object rect) {
		Rect other = (Rect) rect;
		if (getArea() > other.getArea()) {
			return 1;
		} else if (getArea() < other.getArea()) {
			return -1;
		}
		return 0;
	}

	/**
	 * Compares this rect with the given object by ID (must be a rect).
	 * @param obj	the object to compare with.
	 * @return @see java.lang.Object#equals(java.lang.Object)
	 */
	public final boolean equals(final Object obj) {
		if (obj == null) {
			return false;
		} else if (! (obj instanceof Rect)) {
			return false;			
		}
		final Rect o = (Rect) obj;
		if (id == null || o.id == null) {
			return false;
		}
		return o.id.equals(id);
	}

	/**
	 * Gets the ID.
	 * @return the ID.
	 */
	public final String getId() {
		return id;
	}

	/**
	 * Gets the x coordinate.
	 * @return the x.
	 */
	public final int getX() {
		return x;
	}

	/**
	 * Gets the y coordinate.
	 * @return the y.
	 */
	public final int getY() {
		return y;
	}
}
