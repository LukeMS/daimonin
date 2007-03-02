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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * A simple packer to solve a two-dimensional rectangle packing problem. n rectangles are
 * to be packed into one (or more) enclosing rectangle.
 * 
 * This code is based on the files 'BinPacker.hpp' and 'BinPacker.cpp' written by Jesse Anders,
 * who kindly permitted me to use and modify his code.
 * 
 * @author Rumbuff
 */
public class BinPacker {

	/**
	 * Counter of packed rects.
	 */
	private int mNumPacked;

	/**
	 * List of rectangles to be packed.
	 */
	private List<Rect> mRects;

	/**
	 * List of rectangles (packs), which contain the input rectangles when packing is done.
	 */
	private List<Rect> mPacks;

	/**
	 * Root rects.
	 */
	private List<Integer> mRoots;
	
	
	/**
	 * Main, for testing only.
	 * @param args  Command line parameters.
	 */
	public static void main(final String[] args) {
		
		BinPacker packer = new BinPacker();
		
		List<Rect> rects = new ArrayList<Rect>();
		rects.add(new Rect("Rect 01", 14, 5));
		rects.add(new Rect("Rect 02", 9, 3));
		rects.add(new Rect("Rect 03", 5, 11));
		rects.add(new Rect("Rect 04", 3, 8));
		rects.add(new Rect("Rect 10", 11, 2));
		rects.add(new Rect("Rect 07", 5, 4));
		rects.add(new Rect("Rect 08", 2, 6));
		rects.add(new Rect("Rect 05", 6, 3));
		rects.add(new Rect("Rect 06", 3, 6));
		rects.add(new Rect("Rect 09", 4, 2));
		rects.add(new Rect("Rect 11", 4, 2));
		
//		ArrayList<List<Rect>> packs = packer.pack(rects, 30, 20, false);
//		for (int i = 0; i < packs.size(); i++) {
//			System.out.println("  PACK " + i + ":");
//			List<Rect> aPack = packs.get(i);
//			for (int j = 0; j < aPack.size(); j++) {
//				System.out.println(aPack.get(j));
//			}
//		}
		
		int[] dimension = new int[] {1024, 768};
		List<Rect> packsO = packer.packOptimized(rects, false, dimension);
		for (int i = 0; i < packsO.size(); i++) {
			System.out.println(packsO.get(i));
		}
	}

	/**
   * Packs n rectangles into one enclosing rectangle. The size of this rectangle is minimized.
   * 
   * @param rects 		A list of rectangles.
   * @param rotate		when true, the packer is allowed to rotate the rectangles to fit them into the pack.
   * @return A rectangle containing all given rectangles with nearly minimum dimensions.         
   */
	public final List<Rect> packOptimized(final List<Rect> rects, final boolean rotate) {
		return packOptimized(rects, rotate, new int[]{-1, -1});
	}
	
	/**
   * Packs n rectangles into one enclosing rectangle. The size of this rectangle is minimized.
   * 
   * @param rects 			A list of rectangles.
   * @param rotate			when true, the packer is allowed to rotate the rectangles to fit them into the pack.
   * @param dimension 	int-Array with two elements:
   * 										1st: Maximum width of the enclosing rectangle. 
   * 										2nd: Maximum height of the enclosing rectangle.
   * 										Call-by-reference: After successfull packing, these two int store
   * 										the width and height of the enclosing rectangle.
   * @return A rectangle containing all given rectangles with nearly minimum dimensions.         
   */
	public final List<Rect> packOptimized(final List<Rect> rects, final boolean rotate, final int[] dimension) {
		
		// calculate the cumulated width and height values
		int dw = 0;
		int dh = 0;
		int maxWidth = dimension[0];
		int maxHeight = dimension[1];
		
		if (maxWidth == -1 && maxHeight == -1) {
			for (int i = 0; i < rects.size(); i++) {
				dw += rects.get(i).w;
				dh += rects.get(i).h;
			}
		} else {
			dw = maxWidth;
			dh = maxHeight;
		}
		
		// determine the min and max width and height of all rectangles
		int minW = Integer.MAX_VALUE;
		int minH = Integer.MAX_VALUE;
		int maxW = Integer.MIN_VALUE;
		int maxH = Integer.MIN_VALUE;
		for (int i = 0; i < rects.size(); i++) {
			if (rects.get(i).w < minW) {
				minW = rects.get(i).w;
			}
			if (rects.get(i).h < minH) {
				minH = rects.get(i).h;
			}
			if (rects.get(i).w > maxW) {
				maxW = rects.get(i).w;
			}
			if (rects.get(i).h > maxH) {
				maxH = rects.get(i).h;
			}
		}
		
		// translate into 4:3 dimension
//		double oneSeventh = Math.sqrt(dw * dh) / Math.sqrt(12);
//		dw = (int) (oneSeventh * 4);
//		dh = (int) (oneSeventh * 3);

		// pack once to get a first packed rectangle
		ArrayList<List<Rect>> packs = pack(rects, dw, dh, rotate, false);		
		if (packs.size() != 1) {
			if (maxWidth == -1 && maxHeight == -1) {
				throw new RuntimeException("Optimization method needs to get rewritten!!!");
			} else {
				throw new RuntimeException("The given rectangles could not be packed in an"
					+ " enclosing rectangle width these dimensions (" + maxWidth + "x" + maxHeight + ").");
			}
		}
		List<Rect> pack = packs.get(0);
		
		// determine width and height of that rectangle
		dw = Integer.MIN_VALUE;
		dh = Integer.MIN_VALUE;
		for (int i = 0; i < pack.size(); i++) {
			if (pack.get(i).x + pack.get(i).w > dw) {
				dw = pack.get(i).x + pack.get(i).w;
			}
			if (pack.get(i).y + pack.get(i).h > dh) {
				dh = pack.get(i).y + pack.get(i).h;
			}
		}
//		System.out.println("stage 1 : fits in " + dw + ":" + dh);
		
		// reduce w and h at the same time until one pack becomes too small
		int fitsW = dw;
		int fitsH = dh;		
		while (dw - minW >= maxW && dh - minH >= maxH) {
			packs = pack(rects, dw - minW, dh - minH, rotate, false);
			if (packs.size() == 1) {
				pack = packs.get(0);
				dw -= minW;
				dh -= minH;
				fitsW = dw;
				fitsH = dh;
//				System.out.println("stage 2 : fits in " + fitsW + ":" + fitsH);
			} else {
				break;
			}
		}
		
		// try to reduce w
		while (dw - minW >= maxW) {
			packs = pack(rects, dw - minW, dh, rotate, false);
			if (packs.size() == 1) {
				pack = packs.get(0);
				dw -= minW;				
				fitsW = dw;				
//				System.out.println("stage 3w: fits in " + fitsW + ":" + fitsH);
			} else {
				break;
			}
		}
		// try to reduce h
		while (dh - minH >= maxH) {
			packs = pack(rects, dw, dh - minH, rotate, false);
			if (packs.size() == 1) {
				pack = packs.get(0);
				dh -= minH;				
				fitsH = dh;				
//				System.out.println("stage 3h: fits in " + fitsW + ":" + fitsH);
			} else {
				break;
			}
		}
		
//		System.out.println("FINAL   :         " + fitsW + ":" + fitsH);
		dimension[0] = fitsW;
		dimension[1] = fitsH;
		return pack;
	}
	
	/**
   * Packs n rectangles into one or more enclosing rectangles (i.e. packs).
   * If a pack can't take another rectangle (because it's full), another pack is created.
   * 
   * @param rects  					A list of rectangles.
   * @param packSizeW				width of one pack. 
   * @param packSizeH				height of one pack.
   * @param rotate		when true, the packer is allowed to rotate the rectangles to fit them into the pack.
   * @param printID					If the pseudo ID for characterizing the found solution should be printed.
   * @return A list containing one or more packs. Each pack consists of a second list with Rects.         
   */
	public final ArrayList<List<Rect>> pack(final List<Rect> rects, final int packSizeW,
			final int packSizeH, final boolean rotate, final boolean printID) {

		clear(rects);

		// Add rects to member array, and check to make sure none is too big to fit into a pack
		final Rect packRect = new Rect(Rect.ID_DEAULT, packSizeW, packSizeH);
		for (int i = 0; i < rects.size(); i++) {
			if (! fits(rects.get(i), packRect, rotate)) {					
				final String message = "All rectangle dimensions must be <= the pack size. Rectangle with ID '"
					+ rects.get(i).id + "' is too big."; 
				throw new RuntimeException(message);
			} else if (mRects.contains(rects.get(i))) {
				final String message = "There is more than one rectangle with ID '"
					+ rects.get(i).id + "'. This is not allowed, ID's must be unique."; 
				throw new RuntimeException(message);
			} else {
				mRects.add(rects.get(i));
			}
		}

		// Sort from greatest to least area
		sortRects();

		// pack
		while (mNumPacked < mRects.size()) {
			int i = mPacks.size();
			mPacks.add(new Rect(Rect.ID_DEAULT, packSizeW, packSizeH));
			mRoots.add(i);
			fill(i, rotate);
		}
	
		// prepare the return object
		ArrayList<List<Rect>> packs = new ArrayList<List<Rect>>();
		for (int i = 0; i < mRoots.size(); i++) {
			packs.add(new ArrayList<Rect>());
		}
		for (int i = 0; i < mRoots.size(); ++i) {
      addPackToArray(mRoots.get(i), packs.get(i));
		}

		if (printID) {
			printPackedID(packs);
		}
		
		return packs;
	}

	/**
	 * Picks the packed rects and stores them.
	 * @param pack		pack number.
	 * @param array		store for the rects.
	 */
	private void addPackToArray(final int pack, final List<Rect> array) {
		assert (packIsValid(pack));

		int i = pack;
		if (! Rect.ID_DEAULT.equals(mPacks.get(i).id)) {
			Rect myRect = getRectByID(mPacks.get(i).id);
			array.add(new Rect(myRect.id, mPacks.get(i).x, mPacks.get(i).y, myRect.w,
				myRect.h, mPacks.get(i).rotated, myRect.packed));
			
			if (! myRect.packed) {
				System.err.println("Rect with ID=" + myRect.id + " was not packed!");
				assert (false);
			}

			if (mPacks.get(i).getChildren()[0] != -1) {
				addPackToArray(mPacks.get(i).getChildren()[0], array);
			}
			if (mPacks.get(i).getChildren()[1] != -1) {
				addPackToArray(mPacks.get(i).getChildren()[1], array);
			}
		}
	}

	/**
	 * Resets all internal data that may be left from a previous run, including the
	 * rectangles to be packed. 
	 * @param rects  rectangles to be packed.
	 */
	private void clear(final List<Rect> rects) {
		mNumPacked = 0;
		mRects = new ArrayList<Rect>();
		mPacks = new ArrayList<Rect>();
		mRoots = new ArrayList<Integer>();
		resetRects(rects);
	}

	/**
	 * Store rect in lower-left of working area, split, and recurse.
	 * @param pack						pack number.
	 * @param allowRotation		if rect might be rotated to fit better.
	 */
	private void fill(final int pack, final boolean allowRotation) {
		assert (packIsValid(pack));

		int i = pack;

		// For each rect
		for (int j = 0; j < mRects.size(); ++j) {
			// If it's not already packed
			if (!mRects.get(j).packed) {
				// If it fits in the current working area
				if (fits(mRects.get(j), mPacks.get(i), allowRotation)) {
					// Store in lower-left of working area, split, and recurse
					++mNumPacked;
					split(i, j);
					fill(mPacks.get(i).getChildren()[0], allowRotation);
					fill(mPacks.get(i).getChildren()[1], allowRotation);
					return;
				}
			}
		}
	}

	/**
	 * Split the working area either horizontally or vertically with respect to the rect
	 * we're storing, such that we get the largest possible child area.
	 * @param pack	pack number.
	 * @param rect	rect number.
	 */
	private void split(final int pack, final int rect) {
		assert (packIsValid(pack));
		assert (rectIsValid(rect));

		int i = pack;
		int j = rect;

		Rect left = new Rect(mPacks.get(i));
		Rect right = new Rect(mPacks.get(i));
		Rect bottom = new Rect(mPacks.get(i));
		Rect top = new Rect(mPacks.get(i));

		left.y += mRects.get(j).h;
		left.w = mRects.get(j).w;
		left.h -= mRects.get(j).h;
		right.x += mRects.get(j).w;
		right.w -= mRects.get(j).w;		

		bottom.x += mRects.get(j).w;
		bottom.h = mRects.get(j).h;
		bottom.w -= mRects.get(j).w;
		top.y += mRects.get(j).h;
		top.h -= mRects.get(j).h;

		int maxLeftRightArea = left.getArea();
		if (right.getArea() > maxLeftRightArea) {
			maxLeftRightArea = right.getArea();
		}

		int maxBottomTopArea = bottom.getArea();
		if (top.getArea() > maxBottomTopArea) {
			maxBottomTopArea = top.getArea();
		}

		if (maxLeftRightArea > maxBottomTopArea) {
			if (left.getArea() > right.getArea()) {
				mPacks.add(left);
				mPacks.add(right);
			} else {
				mPacks.add(right);
				mPacks.add(left);
			}
		} else {
			if (bottom.getArea() > top.getArea()) {
				mPacks.add(bottom);
				mPacks.add(top);
			} else {
				mPacks.add(top);
				mPacks.add(bottom);
			}
		}

		// This pack area now represents the rect we've just stored, so save the
		// relevant info to it, and assign children.
		mPacks.get(i).w = mRects.get(j).w;
		mPacks.get(i).h = mRects.get(j).h;
		mPacks.get(i).id = mRects.get(j).id;
		mPacks.get(i).rotated = mRects.get(j).rotated;
		mPacks.get(i).setChildren(mPacks.size() - 2, mPacks.size() - 1);

		// Done with the rect
		mRects.get(j).packed = true;
	}
	
	/**
	 * Check to see if rect1 fits in rect2, and rotate rect1 if that will enable it to fit.
	 * @param rect1						rectangle to check.
	 * @param rect2						rectangle to contain rect1 if possible.
	 * @param allowRotation		Allow rotation of rect1 to make it fit.
	 * @return 'true' if rect1 fits in rect2, 'false' otherwise.
	 */
	private boolean fits(final Rect rect1, final Rect rect2, final boolean allowRotation) {
		if (rect1.w <= rect2.w && rect1.h <= rect2.h) {
			return true;
		} else if (allowRotation && rect1.h <= rect2.w && rect1.w <= rect2.h) {
			rect1.rotate();
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Checks if the given rect is valid.
	 * @param i		rect number.
	 * @return		if the given rect is valid.
	 */
	private boolean rectIsValid(final int i) {
		return i >= 0 && i < mRects.size();
	}

	/**
	 * Checks if the given pack is valid.
	 * @param i		pack number.
	 * @return		if the given pack is valid.
	 */
	private boolean packIsValid(final int i) {
		return i >= 0 && i < mPacks.size();
	}

	/**
   * Sorts all rects descending by their area.
   */
	private void sortRects() {
		assert (mRects.size() > 0);

		// sort
		Rect[] sorts = mRects.toArray(new Rect[mRects.size()]);
		Arrays.sort(sorts);
		
		// rebuild m_rects with reverse order of the sorted array
		for (int i = sorts.length - 1; i >= 0; i--) {
			mRects.set(i, sorts[sorts.length - 1 - i]);
		}
	}
	
	/**
	 * Gives a mRect by its ID.
	 * 
	 * @param id  the mRect ID.
	 * @return		the found mRect.
	 */
	private Rect getRectByID(final String id) {
		List<Rect> found = new ArrayList<Rect>();
		for (int i = 0; i < mRects.size(); i++) {
			if (mRects.get(i).id.equals(id)) {
				found.add(mRects.get(i));
			}
		}		
		if (found.size() > 1) {
			System.err.println("More than one rect with ID '" + id + "' found!");
			assert (false);
			return null;
		} else if (found.size() == 0) {
//			System.out.println("No rect with ID '" + ID + "' found!");
			return null;
		} else {
			return found.get(0);
		}
	}
	
	/**
	 * Prints the weighted sums of all x- and y- values of all rectangles in the first pack.
	 * This is a try to get a figure that uniquely identifies a certain pack pattern.
	 * @param packs  the packs.
	 */
	private void printPackedID(final ArrayList<List<Rect>> packs) {
		int id = 0;
		List<Rect> aPack = packs.get(0);
		for (int i = 0; i < aPack.size(); i++) {
			id += (aPack.get(i).x + aPack.get(i).y * 2) * i;
		}
		System.out.println("PackedID of pack 1 is " + id);
	}
	
	/**
	 * Resets the the given rects: x, y, packed, rotated are reset to their defaults.
	 * @param rects  The rectangles to be reset.
	 */
	private void resetRects(final List<Rect> rects) {
		for (int i = 0; i < rects.size(); i++) {
			rects.get(i).x = 0;
			rects.get(i).y = 0;
			rects.get(i).packed = false;
			rects.get(i).rotated = false;
			rects.get(i).setChildren(-1, -1);
		}
	}
}
