using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace test
{
	class Program
	{
		static void Main(string[] args)
		{
			StringBuilder strbuilder = new StringBuilder();
			Console.WriteLine(strbuilder.MaxCapacity);
			string str = "hi\0\0\0\0\0\0\0";
			Console.WriteLine(str.TrimEnd('\0') + "end");
			Console.WriteLine(str.Length);
			//String originalText = "plain text";
			//String key = "key";
			//String en = Encrypt(originalText, key);
			//String de = Decrypt(en, key);

			//Console.WriteLine("Original Text is " + originalText);
			//Console.WriteLine("Encrypted Text is " + en);
			//Console.WriteLine("Decrypted Text is " + de);
		}

		public static string Decrypt(string textToDecrypt, string key)
		{
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			byte[] encryptedData = Convert.FromBase64String(textToDecrypt);
			byte[] pwdBytes = Encoding.UTF8.GetBytes(key);
			byte[] keyBytes = new byte[16];
			int len = pwdBytes.Length;
			if (len > keyBytes.Length)
			{
				len = keyBytes.Length;
			}
			Array.Copy(pwdBytes, keyBytes, len);
			rijndaelCipher.Key = keyBytes;
			rijndaelCipher.IV = keyBytes;
			byte[] plainText = rijndaelCipher.CreateDecryptor().TransformFinalBlock(encryptedData, 0, encryptedData.Length);
			return Encoding.UTF8.GetString(plainText);
		}

		public static string Encrypt(string textToEncrypt, string key)
		{
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			byte[] pwdBytes = Encoding.UTF8.GetBytes(key);
			byte[] keyBytes = new byte[16];
			int len = pwdBytes.Length;
			if (len > keyBytes.Length)
			{
				len = keyBytes.Length;
			}
			Array.Copy(pwdBytes, keyBytes, len);
			rijndaelCipher.Key = keyBytes;
			rijndaelCipher.IV = keyBytes;
			ICryptoTransform transform = rijndaelCipher.CreateEncryptor();
			byte[] plainText = Encoding.UTF8.GetBytes(textToEncrypt);
			return Convert.ToBase64String(transform.TransformFinalBlock(plainText, 0, plainText.Length));
		}
	}
}
