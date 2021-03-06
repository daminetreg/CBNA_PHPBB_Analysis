#include "TopicParser.hpp"
#include "download/HTTPDownloader.hpp"
#include "gumbo-query/src/Document.h"
#include "gumbo-query/src/Node.h"
#include "MessageParser.hpp"

#include <iostream>


TopicParser::TopicParser(const std::string& website):
    website(website)
{
}

bool TopicParser::parse(const int topic_id)
{
    raw_topic.id = topic_id;
    raw_topic.messages.clear();
    for(int page = 0; /*breaked below*/; /*multi-incremented below*/)
    {
        const std::string page_name = "http://" + website + "/t" + std::to_string(topic_id) + "p" + std::to_string(page) +  "-title";

        // download the topic
        HTTPDownloader downloader;
        std::string content = downloader.download(page_name);

        if (content.empty()) break;

        // parse the document
        CDocument doc;
        doc.parse(content.c_str());

        // parse first page data
        if (page == 0)
        {
            if (!parseFirstPage(doc))
                return false;
        }

        // parse post
        CSelection post = doc.find(".post");
        bool nothing_found = true;
        for(int i = 0; i<post.nodeNum(); ++i)
        {
            auto node = post.nodeAt(i);
            MessageParser message_parser;
            if (message_parser.parse(node))
            {
                raw_topic.messages.push_back(message_parser.toRaw());
                ++page;
                nothing_found = false;
            }
        }
        std::cout << "Message read = " << page << '\r' << std::flush;

        if (nothing_found) break;
    }

    return raw_topic.messages.size();
}

bool TopicParser::parseFirstPage(CDocument& doc)
{
    // parse title
    CSelection title = doc.find(".cattitle");
    if (title.nodeNum() == 0) return false;
    raw_topic.title = title.nodeAt(0).text();
    std::cout << raw_topic.id << " | " << raw_topic.title << std::endl;

    // parse path
    CSelection path = doc.find("a.nav span");
    raw_topic.path.clear();
    for(int j = 0; j<path.nodeNum(); ++j)
    {
        //std::cout << j << path.nodeAt(j).text() << std::endl;
        raw_topic.path.push_back(path.nodeAt(j).text());
        if (j>=2) break;
    }
    return true;
}

RawTopic TopicParser::toRaw()
{
    return raw_topic;
}


void TopicParser::reload(const std::string& directory)
{
}
