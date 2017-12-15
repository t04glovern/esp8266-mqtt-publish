/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* List of projects/orgs using your project for the users page */
const users = [
  {
    caption: 'Nathan Glover',
    image: '/esp8266-mqtt-publish/img/nathanglover.png',
    infoLink: 'https://nathanglover.com/',
    pinned: true,
  },
];

const siteConfig = {
  title: 'ESP8266 MQTT' /* title for your website */,
  tagline: 'Documentation for the ESP8266 MQTT firmware',
  url: 'https://t04glovern.github.io' /* your website url */,
  baseUrl: '/esp8266-mqtt-publish/' /* base url for your project */,
  projectName: 'esp8266-mqtt-publish',
  headerLinks: [
    {doc: 'readme', label: 'Readme'},
    {blog: true, label: 'Blog'},
  ],
  users,
  /* path to images for header/footer */
  headerIcon: 'img/esp8266-chip.png',
  footerIcon: 'img/esp8266-chip.png',
  favicon: 'img/favicon/favicon.ico',
  /* colors for website */
  colors: {
    primaryColor: '#2E8555',
    secondaryColor: '#205C3B',
  },
  // This copyright info is used in /core/Footer.js and blog rss/atom feeds.
  copyright:
    'Copyright © ' +
    new Date().getFullYear() +
    ' t04glovern',
  organizationName: 't04glovern', // or set an env variable ORGANIZATION_NAME
  projectName: 'esp8266-mqtt-publish', // or set an env variable PROJECT_NAME
  highlight: {
    // Highlight.js theme to use for syntax highlighting in code blocks
    theme: 'default',
  },
  scripts: ['https://buttons.github.io/buttons.js'],
  // You may provide arbitrary config keys to be used as needed by your template.
  repoUrl: 'https://github.com/t04glovern/esp8266-mqtt-publish',
};

module.exports = siteConfig;
