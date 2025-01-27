﻿using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace PenguLoader.Main
{
    static class Config
    {
        public static string ConfigPath => GetPath("config");
        public static string DataStorePath => GetPath("datastore");
        public static string AssetsDir => GetPath("assets");
        public static string PluginsDir => GetPath("plugins");

        static Config()
        {
            Utils.EnsureDirectoryExists(AssetsDir);
            Utils.EnsureDirectoryExists(PluginsDir);
            Utils.EnsureFileExists(ConfigPath);
            Utils.EnsureFileExists(DataStorePath);
        }

        public static string Language
        {
            get => Get("Language");
            set => Set("Language", value);
        }

        public static bool OptimizeClient
        {
            get => GetBool("OptimizeClient", true);
            set => SetBool("OptimizeClient", value);
        }

        public static bool SuperLowSpecMode
        {
            get => GetBool("SuperLowSpecMode", false);
            set => SetBool("SuperLowSpecMode", value);
        }

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern long WritePrivateProfileString(string section, string key, string value, string file);

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileString(string section, string key, string @default, StringBuilder retval, int size, string file);

        private static string GetPath(string folderName)
        {
            return Path.Combine(Directory.GetCurrentDirectory(), folderName);
        }

        private static string Get(string key, string @default = "")
        {
            var value = new StringBuilder(2048);
            GetPrivateProfileString("Main", key, @default, value, 2048, ConfigPath);
            return value.ToString();
        }

        private static void Set(string key, string value) => WritePrivateProfileString("Main", key, value, ConfigPath);

        private static int GetInt(string key, int @default = 0) => int.TryParse(Get(key), out int result) ? result : @default;

        static bool GetBool(string key, bool @default)
        {
            var value = Get(key);
            if (value == "true" || value == "1")
                return true;
            else if (value == "false" || value == "0")
                return false;

            return @default;
        }

        static void SetBool(string key, bool value) => Set(key, value ? "true" : "false");
    }
}