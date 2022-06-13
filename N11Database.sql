-- phpMyAdmin SQL Dump
-- version 5.1.1deb5ubuntu1
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Jun 13, 2022 at 12:29 AM
-- Server version: 8.0.29-0ubuntu0.22.04.2
-- PHP Version: 8.1.2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `N11`
--

-- --------------------------------------------------------

--
-- Table structure for table `phones`
--

CREATE TABLE `phones` (
  `id` varchar(64) NOT NULL,
  `phone` varchar(64) NOT NULL,
  `uuid` varchar(16) NOT NULL,
  `code` varchar(4) DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `phones`
--

INSERT INTO `phones` (`id`, `phone`, `uuid`, `code`, `enabled`) VALUES
('41c2e74c7d1ed7bb4a3b53e4f8dede31c445fb17de08b87befb775f99f8fd40c', '(806) 662-8337', 'd4a6b3a9b8b30789', '6864', 1),
('7c2be0ff7866f32b4200ed57e9a3c7098a1f3e5fffe6d6b1707ba63ba9b5fcf9', '(806) 663-0843', 'd4a6b3a9b8b30789', '2335', 1);

-- --------------------------------------------------------

--
-- Table structure for table `sessions`
--

CREATE TABLE `sessions` (
  `id` varchar(128) NOT NULL,
  `tokens` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tokens`
--

CREATE TABLE `tokens` (
  `token` varchar(32) NOT NULL,
  `uuid` varchar(16) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `os` text,
  `browser` text,
  `date` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `tokens`
--

INSERT INTO `tokens` (`token`, `uuid`, `ip`, `os`, `browser`, `date`) VALUES
('1979d8e495d73334565b19f34a3df631', 'd4a6b3a9b8b30789', '127.0.0.1', 'Windows', 'Chrome', '06/07/2022 11:16:22');

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `uuid` varchar(16) NOT NULL,
  `email` text NOT NULL,
  `name` text NOT NULL,
  `password` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`uuid`, `email`, `name`, `password`) VALUES
('8608ffa4ced15a83', 'test@gmail.com', 'Test Dummy', 'c8157cc2a8012e888ad755034eed997d5d4355066a480b8827e857eddcaf7281d7050eb28ec7c358ec4e756984da96dcf7cbe0e1632c6ac9d5eb8aefb20f4cd4'),
('d4a6b3a9b8b30789', 'levicowan2005@icloud.com', 'Levi Hicks', 'c8157cc2a8012e888ad755034eed997d5d4355066a480b8827e857eddcaf7281d7050eb28ec7c358ec4e756984da96dcf7cbe0e1632c6ac9d5eb8aefb20f4cd4');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `phones`
--
ALTER TABLE `phones`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `sessions`
--
ALTER TABLE `sessions`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tokens`
--
ALTER TABLE `tokens`
  ADD PRIMARY KEY (`token`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`uuid`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
