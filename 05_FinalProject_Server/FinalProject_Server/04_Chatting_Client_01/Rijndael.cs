using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace _04_Chatting_Client_01
{
	class Rijndael
	{
		static byte[] nonce = new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
		static byte[] counter = new byte[Macro.SIZE_CIPHER_ELEMENT] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		static byte[] iv = new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

		private static byte[] countermode(byte[] buffer, int offset, int length, byte[] key)
		{
			// CTR = ECB(counter) + xor
			if (length < 1)
				return null;
			
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.ECB;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;

			ICryptoTransform transform = rijndaelCipher.CreateEncryptor();

			byte[] counter_cipher;
			byte[] buffer_ret = new byte[length];

			Console.Write("\t[1][" + length + "] -> ");
			for (int i = offset; i < offset + length; i++)
				Console.Write(string.Format("{0:x2} ", buffer[i]));
			Console.WriteLine("");
			
			for (int i = 0; i < length; i+= Macro.SIZE_CIPHER_ELEMENT)
			{
				// counter 증가
				counter[Macro.SIZE_CIPHER_ELEMENT - 1]++;

				// counter ECB 암호화
				counter_cipher = transform.TransformFinalBlock(counter, 0, Macro.SIZE_CIPHER_ELEMENT);
				Console.Write("\t[3][" + length + "] -> ");
				for (int k = 0; k < Macro.SIZE_CIPHER_ELEMENT; k++)
					Console.Write(string.Format("{0:x2} ", counter_cipher[k]));
				Console.WriteLine("");

				// counter_cipher xor buffer
				for (int j = 0; i + j < length && j < Macro.SIZE_CIPHER_ELEMENT; j++)
				{
					buffer_ret[i + j] = (byte)(buffer[i + j + offset] ^ counter_cipher[j]);
				}
			}
			counter[Macro.SIZE_CIPHER_ELEMENT - 1] = 0;
			Console.Write("\t[2][" + length + "] -> ");
			for (int i = 0; i < length; i++)
				Console.Write(string.Format("{0:x2} ", buffer_ret[i]));
			Console.WriteLine("");

			return buffer_ret;
		}

		private static byte[] encryptCbcmode(byte[] buffer, int offset, int length, byte[] key)
		{
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;
			rijndaelCipher.IV = iv;
			ICryptoTransform transform = rijndaelCipher.CreateEncryptor();
			return transform.TransformFinalBlock(buffer, offset, length);
		}
		private static byte[] _decryptCbcmode(byte[] buffer, int offset, int length, byte[] key)
		{
			if (length < 1)
				return null;

			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;
			rijndaelCipher.IV = iv;

			ICryptoTransform transform = rijndaelCipher.CreateDecryptor();
			return transform.TransformFinalBlock(buffer, offset, length);
		}
		private static byte[] decryptCbcmode(byte[] buffer, int offset, int length, byte[] key)
		{
			if (length < 1)
				return null;

			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;
			rijndaelCipher.IV = iv;

			byte[] tmp = new byte[Macro.SIZE_BUFFER];
			int _offset = offset;
			int _length = Macro.SIZE_CIPHER_ELEMENT;
			int size_ret = 0;
			for (int i=0; i<length; i+=Macro.SIZE_CIPHER_ELEMENT)
			{
				byte[] plain;
				try
				{
					ICryptoTransform transform = rijndaelCipher.CreateDecryptor();
					plain = transform.TransformFinalBlock(buffer, _offset, _length);
				}
				catch(CryptographicException ex)
				{
					_length += Macro.SIZE_CIPHER_ELEMENT;
					continue;

					throw ex;
				}
				_offset += _length;
				_length = Macro.SIZE_CIPHER_ELEMENT;
				Array.Copy(plain, 0, tmp, size_ret, plain.Length);
				size_ret += plain.Length;
			}

			byte[] ret = new byte[size_ret];
			Array.Copy(tmp, ret, size_ret);
			return ret;

			//ICryptoTransform transform = rijndaelCipher.CreateDecryptor();
			//ret = transform.TransformFinalBlock(buffer, offset, length);
			//return ret;
		}

		public static byte[] decrypt(byte[] buffer, int offset, int length, byte[] key)
		{
			return decryptCbcmode(buffer, offset, length, key);
			//return countermode(buffer, offset, length, key);
		}
		public static byte[] encrypt(byte[] buffer, int offset, int length, byte[] key)
		{
			return encryptCbcmode(buffer, offset, length, key);
			//return countermode(buffer, offset, length, key);
		}
	}
}
