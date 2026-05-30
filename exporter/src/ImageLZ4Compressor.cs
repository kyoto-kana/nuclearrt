using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// NuGet: K4os.Compression.LZ4
using K4os.Compression.LZ4;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

public static class ImageLZ4Compressor
{
	// little endian 
	private const uint SDL_PIXELFORMAT_ARGB8888 = 0x16362004;

	public static void ExportToLZ4(Bitmap source, Stream output, bool useHC = false)
	{
		int width = source.Width;
		int height = source.Height;
		int pitch = width * 4;

		var converted = source.Clone(
			new Rectangle(0, 0, width, height),
			PixelFormat.Format32bppArgb);

		BitmapData bmpData = converted.LockBits(
			new Rectangle(0, 0, width, height),
			ImageLockMode.ReadOnly,
			PixelFormat.Format32bppArgb);

		byte[] rawPixels = new byte[pitch * height];
		Marshal.Copy(bmpData.Scan0, rawPixels, 0, rawPixels.Length);
		converted.UnlockBits(bmpData);
		converted.Dispose();

		byte[] compressed = new byte[LZ4Codec.MaximumOutputSize(rawPixels.Length)];
		int compLen = LZ4Codec.Encode(
			rawPixels, 0, rawPixels.Length,
			compressed, 0, compressed.Length,
			useHC ? LZ4Level.L12_MAX : LZ4Level.L00_FAST);

		using var bw = new BinaryWriter(output, System.Text.Encoding.UTF8, leaveOpen: true);
		bw.Write((ushort)width);
		bw.Write((ushort)height);
		bw.Write(SDL_PIXELFORMAT_ARGB8888);
		bw.Write((int)compLen);
		bw.Write(compressed, 0, compLen);
	}
	public static byte[] CompressMaskLZ4(byte[] rawMask, int width, int height)
	{
		byte[] compressed = new byte[LZ4Codec.MaximumOutputSize(rawMask.Length)];
		int compressedSize = LZ4Codec.Encode(
			rawMask,
			0,
			rawMask.Length,
			compressed,
			0,
			compressed.Length,
			LZ4Level.L12_MAX
		);

		using var ms = new MemoryStream();
		using var bw = new BinaryWriter(ms);

		bw.Write((ushort)width);
		bw.Write((ushort)height);
		bw.Write(rawMask.Length);
		bw.Write(compressedSize);
		bw.Write(compressed, 0, compressedSize);

		return ms.ToArray();
	}
}
