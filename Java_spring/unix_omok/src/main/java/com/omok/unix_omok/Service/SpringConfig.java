package com.omok.unix_omok.Service;


import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import com.omok.unix_omok.repository.OmokDataRepository;

@Configuration
public class SpringConfig {

    private OmokDataRepository omokDataRepository;

    @Autowired
    public SpringConfig(OmokDataRepository omokDataRepository) {
        this.omokDataRepository = omokDataRepository;
    }

    @Bean
    public OmokDataService omokDataService() {
        return new OmokDataService(omokDataRepository);
    }
}